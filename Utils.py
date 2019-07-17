from typing import List
from pytorch_pretrained_bert import BertTokenizer


def padSentences(sentIndices: List[List[int]], padTokenIdx: int):
    maxLen = max([len(s) for s in sentIndices])
    return [s + [padTokenIdx] * (maxLen - len(s)) for s in sentIndices]


def convertTokensToIds(tokenizer: BertTokenizer, sentence: List[str]):
    """
    converts a single sentence of tokens to token IDs
    :param tokenizer: BertTokenizer to be used to tokenize
    :param sentence: tokenized sentence to be converted to IDs
    :return: token Ids of the sentence
    """
    ids = []
    for token in sentence:
        try:
            i = tokenizer.vocab[token]
        except KeyError:
            # if character is not found in vocab, then mark it [UNK]
            i = tokenizer.vocab['[UNK]']
        ids.append(i)
    return ids


def countLinesInFile(filepath: str):
    lines = 0
    with open(filepath, 'r') as f:
        for _ in f:
            lines += 1
    return lines
