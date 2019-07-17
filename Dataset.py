"""
Provides Dataset enclosure for representing patent training/dev/evaluation data

Usage:
    Dataset.py --tokenizer=<file>
"""

import torch
import pandas as pd
import numpy as np

from typing import Optional, List
from torch.utils import data
from pytorch_pretrained_bert import BertTokenizer

from Utils import countLinesInFile


class LargePatentDataset(data.Dataset):
    def __init__(self, filePath: str, size: int, tokenizer: BertTokenizer,
                 chunkSize=2**16, shuffle=False):
        self.filePath: str = filePath
        self.tokenizer: BertTokenizer = tokenizer
        self.size: int = size
        self.chunkSize: int = chunkSize
        self.shuffle: bool = shuffle

        # data properties
        self.Xs: Optional[np.ndarray] = None        # tokenized sentences
        self.Ys: Optional[np.ndarray, List] = None  # tags for each sentence

        # flags
        self.loadStarted: bool = False

    def __len__(self):
        return self.size

    def __getitem__(self, item):
        raise NotImplementedError()

    def _chunkLoad(self):
        raise NotImplementedError()

    def _shuffle(self):
        randIdxs = torch.randperm(self.chunkSize)
        self.Xs = self.Xs[randIdxs]
        self.Ys = self.Ys[randIdxs]
        return randIdxs

    @staticmethod
    def collate(batch):
        raise NotImplementedError()


class CsvDataset(LargePatentDataset):
    """
    Large scale on-the-fly CSV Dataset representation. Only supports **sequential reading**
    Undefined behavior will happen if the dataset is read non-sequentially
    """
    def __init__(self, filePath: str, size: int, tokenizer: BertTokenizer,
                 chunkSize=2**16, shuffle=False):
        super().__init__(filePath, size, tokenizer, chunkSize, shuffle)
        # additional data property for indicating unique ids for each data entry
        self.ids: Optional[np.ndarray] = None

    def _chunkLoad(self):
        """
        returns an iterator that yields each chunk of the CSV file
        :yield:
            (np.ndarray) ids: unique ID for each data entry
            (np.ndarray) Xs: tokenized sentences for each data entry
            (np.ndarray) Ys: string tags for each sentences
        """
        allTags = None
        for chunk in pd.read_csv(self.filePath, chunksize=self.chunkSize):
            if allTags is None:
                allTags = chunk.columns.values[2:]
            chunkNp: np.ndarray = chunk.values
            ids: np.ndarray = chunkNp[:, 0]
            Xs: np.ndarray = np.array([self.tokenizer.tokenize(sent) for sent in chunkNp[:, 1]])
            Ys: List = [[] for _ in range(self.chunkSize)]
            tagIds = np.nonzero(chunkNp[:, 2:])

            for i in range(len(tagIds[0])):
                Ys[tagIds[0][i]].append(allTags[tagIds[1][i]])

            Ys: np.ndarray = np.array(Ys)

            yield ids, Xs, Ys

    def _shuffle(self):
        randIdxs = super()._shuffle()
        self.ids = self.ids[randIdxs]
        return randIdxs

    def __getitem__(self, index):
        """
        :param index:
        :return (List[str], List[str]]): a tokenized sentence and a list of tags
        """
        if not self.loadStarted:
            self.dataGenerator = self._chunkLoad()
            self.loadStarted = True
        if index % self.chunkSize == 0:
            self.ids, self.Xs, self.Ys = next(self.dataGenerator)
            # shuffle the dataset on load if shuffle is True
            if self.shuffle:
                self._shuffle()

        return self.Xs[index % self.chunkSize], self.Ys[index % self.chunkSize]

    @staticmethod
    def collate(batch):
        return zip(*sorted(batch, key=lambda e: len(e[0]), reverse=True))


class TsvDataset(LargePatentDataset):
    def __init__(self, filePath: str, size: int, tokenizer: BertTokenizer,
                 chunkSize=2**16, shuffle=False):
        super().__init__(filePath, size, tokenizer, chunkSize, shuffle)

    def _chunkLoad(self):
        """
        returns an iterator that yields each chunk of the TSV file
        :yield:
            (np.ndarray) Xs: input (abstract) converted to tokens for each data entry
            (np.ndarray) Ys: output (tags) for each data entry
        """
        for chunk in pd.read_csv(self.filePath, sep='\t', chunksize=self.chunkSize,
                                 header=None, names=['tag', 'text']):
            chunkNp: np.ndarray = chunk.values
            Xs: np.ndarray = np.array([self.tokenizer.tokenize(sent) for sent in chunkNp[:, 1]])
            Ys: np.ndarray = chunkNp[:, 0]

            yield Xs, Ys

    def __getitem__(self, index):
        """
        :param index:
        :return (List[str]], str): a tokenized sentence and a single tag
        """
        if not self.loadStarted:
            self.dataGenerator = self._chunkLoad()
            self.loadStarted = True

        if index % self.chunkSize == 0:
            self.Xs, self.Ys = next(self.dataGenerator)
            # shuffle the dataset on load if shuffle is True
            if self.shuffle:
                self._shuffle()

        return self.Xs[index % self.chunkSize], self.Ys[index % self.chunkSize]

    @staticmethod
    def collate(batch):
        return zip(*sorted(batch, key=lambda e: len(e[0]), reverse=True))


if __name__ == "__main__":
    tokenizer = BertTokenizer.from_pretrained("/home/yuan/Documents/bert/bert-base-chinese")
    csvFile = '/media/yuan/Samsung_T5/Documents/patent/100-line-training.csv'
    lines = countLinesInFile(csvFile)
    ds = CsvDataset(csvFile, size=lines-1, tokenizer=tokenizer, chunkSize=5)

    for i in range(len(ds)):
        xs, ys = ds[i]
        print(xs)
        print(ys)

    tsvFile = '/media/yuan/Samsung_T5/Documents/patent/dev_test.tsv'
    lines = countLinesInFile(tsvFile)
    ts = TsvDataset(tsvFile, size=lines, tokenizer=tokenizer, chunkSize=10)

    for i in range(len(ts)):
        xs, ys = ts[i]
        print(xs)
        print(ys)

