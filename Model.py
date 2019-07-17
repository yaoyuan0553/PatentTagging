from pytorch_pretrained_bert.tokenization import BertTokenizer, WordpieceTokenizer
from pytorch_pretrained_bert.modeling import BertForPreTraining, BertPreTrainedModel, BertModel, BertConfig, \
    BertForMaskedLM, BertForSequenceClassification
from pathlib import Path
import torch
import re
from torch import Tensor
from torch.nn import BCEWithLogitsLoss
from fastai.text import Tokenizer, Vocab
import pandas as pd
import collections
import os
import pdb
from tqdm import tqdm, trange
import sys
import random
import numpy as np
import apex
import json
from sklearn.model_selection import train_test_split

module_path = os.path.abspath(os.path.join('..'))
if module_path not in sys.path:
    sys.path.append(module_path)

from sklearn.metrics import roc_curve, auc

from torch.utils.data import TensorDataset, DataLoader, RandomSampler, SequentialSampler
from torch.utils.data.distributed import DistributedSampler
from pytorch_pretrained_bert.optimization import BertAdam

import logging

logging.basicConfig(format='%(asctime)s - %(levelname)s - %(name)s -   %(message)s',
                    datefmt='%m/%d/%Y %H:%M:%S',
                    level=logging.INFO)
logger = logging.getLogger(__name__)

DATA_PATH = Path('/home/ky/text_classification/data/')

PATH = Path('/home/ky/text_classification/data/tmp')
CLAS_DATA_PATH = PATH / 'class'
model_state_dict = None
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

config_a = BertConfig.from_json_file(config_path)
print(isinstance(config_a, BertConfig))


class BertForMultiLabelSequenceClassification(BertPreTrainedModel):

    def __init__(self, config=config_a, num_labels=627):
        super(BertForMultiLabelSequenceClassification, self).__init__(config)
        self.num_labels = num_labels
        self.bert = BertModel(config)
        self.dropout = torch.nn.Dropout(config.hidden_dropout_prob)
        self.classifier = torch.nn.Linear(config.hidden_size, num_labels)
        self.apply(self.init_bert_weights)

    def forward(self, input_ids, token_type_ids=None, attention_mask=None, labels=None):
        _, pooled_output = self.bert(input_ids, token_type_ids, attention_mask, output_all_encoded_layers=False)
        pooled_output = self.dropout(pooled_output)
        logits = self.classifier(pooled_output)

        if labels is not None:
            loss_fct = BCEWithLogitsLoss()
            loss = loss_fct(logits.view(-1, self.num_labels), labels.view(-1, self.num_labels))
            return loss
        else:
            return logits

    def freeze_bert_encoder(self):
        for param in self.bert.parameters():
            param.requires_grad = False

    def unfreeze_bert_encoder(self):
        for param in self.bert.parameters():
            param.requires_grad = True

model_path = '/home/ky/text_classification/BLUC_MODEL/trained_model/pytorch_model.bin'

model = BertForMultiLabelSequenceClassification()
model.load_state_dict(torch.load(model_path))


