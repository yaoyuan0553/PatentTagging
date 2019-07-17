

from pytorch_pretrained_bert import BertConfig
from pathlib import Path

from Model import BertForMultiLabelSequenceClassification


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

    config_a = BertConfig.from_json_file(config_path)

    model_path = '/home/ky/text_classification/BLUC_MODEL/trained_model/pytorch_model.bin'
    model = BertForMultiLabelSequenceClassification.load(model_path, config_a, 627)


if __name__ == '__main__':
    main()
