import torch
import sys

from torch.utils import data
from Dataset import TsvDataset
from Model import BertForMultiLabelSequenceClassification


class Predictor(object):

    def __init__(self, model: BertForMultiLabelSequenceClassification, device: torch.device):
        self.model = model
        self.device = device

        # put model on GPU/CPU
        self.model.to(device)
        # set model to evaluation mode
        self.model.eval()
