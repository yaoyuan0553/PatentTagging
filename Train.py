import torch

from Model import BertForMultiLabelSequenceClassification
from Dictionary import TagDictionary


class Trainer(object):

    def __init__(self, model: BertForMultiLabelSequenceClassification,
                 tagDict: TagDictionary, device: torch.device, maxInputLen = 512):
        self.model = model
        self.tagDict = tagDict
        self.device = device
        self.maxInputLen = maxInputLen

        self.model.to(device)

