
import torch

from pytorch_pretrained_bert import BertConfig
from pathlib import Path

from Model import BertForMultiLabelSequenceClassification
from Dictionary import TagDictionary
from Dataset import TsvDataset
from pytorch_pretrained_bert import BertTokenizer
from Predict import Predictor


def main():

    DATA_PATH = Path('/home/ky/text_classification/data/')

    PATH = Path('/home/ky/text_classification/data/tmp')
    CLAS_DATA_PATH = PATH / 'class'
    BERT_PRETRAINED_PATH = Path('/home/ky/text_classification/chinese_L-12_H-768_A-12/')
    PYTORCH_PRETRAINED_BERT_CACHE = BERT_PRETRAINED_PATH / 'cache/'
    PYTORCH_PRETRAINED_BERT_CACHE.mkdir(exist_ok=True)

    args = {
        "train_size": -1,
        "val_size": -1,
        "full_data_dir": DATA_PATH,
        "data_dir": PATH,
        "task_name": "toxic_multilabel",
        "bert_model": BERT_PRETRAINED_PATH,
        "output_dir": CLAS_DATA_PATH / 'output',
        "max_seq_length": 512,
        "do_train": True,
        "do_eval": True,
        "do_lower_case": True,
        "train_batch_size": 32,
        "eval_batch_size": 32,
        "learning_rate": 3e-5,
        "num_train_epochs": 4.0,
        "warmup_proportion": 0.1,
        "no_cuda": True,
        "local_rank": -1,
        "seed": 42,
        "gradient_accumulation_steps": 1,
        "optimize_on_cpu": False,
        "fp16": False,
        "loss_scale": 128
    }

    config_path = '/home/ky/text_classification/BLUC_MODEL/trained_model/config.json'
    model_path = '/home/ky/text_classification/BLUC_MODEL/trained_model/pytorch_model.bin'
    tag_path = '/home/ky/text_classification/data/tmp/labels.json'
    dataset_path = '/home/ky/text_classification/data/dev-20000.tsv'
    bert_pretrained_path = '/data/disk1/SegPOSdata/Bert_Pretrained/bert-base-chinese'

    config_a = BertConfig.from_json_file(config_path)

    device = torch.device('cuda:1')
    print(device)

    tokenizer = BertTokenizer.from_pretrained(bert_pretrained_path)
    dataset = TsvDataset(dataset_path, tokenizer, chunkSize=2**12)
    tagDict = TagDictionary.load(tag_path)
    model = BertForMultiLabelSequenceClassification.load(model_path, config_a, 627)
    predictor = Predictor(model, tagDict, device, maxInputLen=512)
    acc = predictor.validate(dataset, args['eval_batch_size'])

    print(len(dataset))
    print(acc)


if __name__ == '__main__':
    main()
