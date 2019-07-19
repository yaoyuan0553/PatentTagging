import torch
import logging
import sys
import os

from pytorch_pretrained_bert.modeling import BertPreTrainedModel, BertModel, BertConfig
from torch.nn import BCEWithLogitsLoss

from typing import Optional

module_path = os.path.abspath(os.path.join('..'))

if module_path not in sys.path:
    sys.path.append(module_path)


logging.basicConfig(format='%(asctime)s - %(levelname)s - %(name)s -   %(message)s',
                    datefmt='%m/%d/%Y %H:%M:%S',
                    level=logging.INFO)
logger = logging.getLogger(__name__)


class BertForMultiLabelSequenceClassification(BertPreTrainedModel):

    def __init__(self, config, num_labels):
        super(BertForMultiLabelSequenceClassification, self).__init__(config)
        self.num_labels = num_labels
        self.bert = BertModel(config)
        self.dropout = torch.nn.Dropout(config.hidden_dropout_prob)
        self.classifier = torch.nn.Linear(config.hidden_size, num_labels)
        self.apply(self.init_bert_weights)

    @property
    def device(self) -> torch.device:
        return self.classifier.weight.device

    def forward(self, input_ids: torch.Tensor, token_type_ids=None,
                attention_mask: Optional[torch.Tensor] = None, labels = None):
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

    @classmethod
    def load(cls, modelpath, config, num_labels):
        print('loading model from [%s]' % modelpath, file=sys.stderr)
        model = cls(config, num_labels)
        model.load_state_dict(torch.load(modelpath))
        return model
