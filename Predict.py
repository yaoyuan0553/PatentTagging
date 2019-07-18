import torch
import sys
import numpy as np

from torch.utils import data
from tqdm import tqdm

from Dataset import TsvDataset, LargePatentDataset
from Model import BertForMultiLabelSequenceClassification
from Dictionary import TagDictionary
from Utils import convertTokensToIds, padBatchTokenIds


class Predictor(object):

    def __init__(self, model: BertForMultiLabelSequenceClassification,
                 tagDict: TagDictionary, device: torch.device, maxInputLen = 512):
        self.model = model
        self.tagDict = tagDict
        self.device = device
        self.maxInputLen = maxInputLen

        # put model on GPU/CPU
        self.model.to(device)
        # set model to evaluation mode
        self.model.freeze_bert_encoder()
        self.model.eval()

    def batchPredict(self, dataset: LargePatentDataset, batchSize):
        batchSampler = data.BatchSampler(data.SequentialSampler(dataset),
                                         batch_size=batchSize, drop_last=False)
        dataGenerator = data.DataLoader(dataset, batch_sampler=batchSampler,
                                        collate_fn=dataset.collate)
        with torch.no_grad():
            with tqdm(total=len(dataset), unit='st', unit_scale=True, file=sys.stdout) as t:
                t.set_description('Predicting')
                for sentsTokens, batchGoldTags in dataGenerator:
                    actualBatchLen = len(sentsTokens)
                    # truncate sentence to maxInputLen (-2 to account for [CLS] & [SEP] tokens)
                    sentsTokens = [["[CLS]"] + sent[:self.maxInputLen-2] + ["[SEP]"] for sent in sentsTokens]
                    sentsTokenIds = [
                        convertTokensToIds(sent, '[UNK]', lambda token: dataset.tokenizer.vocab[token])
                        for sent in sentsTokens]
                    paddedSentsTokenIds = torch.tensor(padBatchTokenIds(sentsTokenIds, 0, len(sentsTokenIds[0])),
                                                       dtype=torch.long, device=self.device)
                    attentionMask = torch.tensor([[float(i > 0) for i in sent] for sent in paddedSentsTokenIds],
                                                 device=self.device)
                    batchLogits = self.model(input_ids=paddedSentsTokenIds, attention_mask=attentionMask).sigmoid()
                    batchLogits: np.ndarray = batchLogits.detach().cpu().numpy()
                    # print(batchLogits)
                    maxIndices = batchLogits.argmax(axis=1)
                    # print(maxIndices)
                    batchPredTags = self.tagDict.indices2words(maxIndices)
                    t.update(actualBatchLen)

                    yield batchPredTags, batchGoldTags

    def predict(self, dataset: LargePatentDataset, batchSize = 256):
        predTags = []
        for batchPredTags, _ in self.batchPredict(dataset, batchSize):
            predTags.extend(batchPredTags)

        return predTags

    def validate(self, dataset: LargePatentDataset, batchSize = 256):
        correctCount = 0
        totalCount = 0
        for batchPredTags, batchGoldTags in self.batchPredict(dataset, batchSize):
            assert len(batchPredTags) == len(batchGoldTags), \
                'prediction & golden batch length do not match!'

            totalCount += len(batchGoldTags)
            for i in range(len(batchGoldTags)):
                correctCount = correctCount + 1 if batchPredTags[i] == batchGoldTags[i] else correctCount
        return correctCount / totalCount

