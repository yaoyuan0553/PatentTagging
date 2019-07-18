import torch
import json
import sys

from typing import List, Iterable

from Utils import padBatchTokenIds


class TagDictionary(object):
    def __init__(self, padding=False):
        """
        :param Iterable wordEmbeddings: array-like type, should support append() function
        """
        self.word2idx = dict()
        self.idx2word = []
        self.unkId = -1
        if padding:
            self.word2idx['[PAD]'] = 0  # pad token and unknown token
            self.idx2word.append(['[PAD]'])
            # self.unkId = self.word2idx['[PAD]']
        # self.idx2word = {v: k for k, v in self.word2idx.items()}

    def __getitem__(self, word: str):
        return self.word2idx.get(word, self.unkId)

    def __contains__(self, word: str):
        return word in self.word2idx

    def __setitem__(self, key, value):
        raise ValueError('forbidden to change existing entries')

    def __len__(self):
        return len(self.word2idx)

    def __repr__(self):
        return 'Dictionary[size=%d]' % len(self)

    def add(self, word: str):
        """
        :param str word: word to be added
        """
        if word not in self:
            idx = self.word2idx[word] = len(self)
            self.idx2word.append(word)
            return idx
        else:
            print("duplicate word %s" % word, file=sys.stderr)
            return self[word]

    def words2indices(self, sents: Iterable):
        return [[self[w] for w in s] for s in sents]

    def indices2words(self, wordidxs: Iterable):
        return [self.idx2word[i] for i in wordidxs]

    def convertToInputTensor(self, sents: List[List[str]], device: torch.device,
                             batchFirst=False) -> torch.tensor:
        """
        Convert a list of sentences into tensor with necessary padding for shorter sentences.
        :param List[List[str]] sents: list of sentences of words
        :param torch.device device: device on which to load the tensor, i.e. sys RAM or GPU
        :param bool batchFirst: if True, then [batch, ids]

        :return sentsIdxs: tensor of shape (maxSentLength, batchSize)
        """
        idxs = self.words2indices(sents)
        sentsIdxs = torch.tensor(padBatchTokenIds(idxs, self['[PAD]']), dtype=torch.long, device=device)
        if batchFirst:
            return sentsIdxs
        return torch.t(sentsIdxs)

    def save(self, filepath: str):
        json.dump(self.idx2word, open(filepath, 'w'))

    @classmethod
    def load(cls, filepath: str):
        tags = json.load(open(filepath, 'r'))
        tagDict = cls()
        for tag in tags:
            tagDict.add(tag)
        return tagDict


if __name__ == '__main__':
    tagDict = TagDictionary.load('/home/ky/text_classification/data/tmp/labels.json')
    print(tagDict)
    tagDict.save('/home/yy/Documents/BLUC-Reimplement/test.json')
    device = torch.device('cpu')
    sents = [['A01L',
              'G16Z',
              'F21H',
              'C10F',
              'G06E',
              'A99Z',
              ],
             ["F17B", "B60S", "D02G", "G10G", "B43M", "C01G", "F04D", "B01J", "A47C", "C05D", "G04B", "B01L", "F16K"]]
    print(tagDict.convertToInputTensor(sents, device, batchFirst=True))
