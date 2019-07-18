from typing import List, Callable


def padBatchTokenIds(batchTokenIds: List[List[int]], padTokenIdx: int, maxLen=None):
    if maxLen is None:
        maxLen = max([len(s) for s in batchTokenIds])
    return [s + [padTokenIdx] * (maxLen - len(s)) for s in batchTokenIds]


def convertTokensToIds(sentence: List[str], unkToken: str,
                       token2IdFunc: Callable[[str], int]):
    """
    converts a single sentence of tokens to token IDs
    :param (List[str]) sentence: tokenized sentence to be converted to IDs
    :param (str) unkToken: BertTokenizer to be used to tokenize
    :param (Callable[[str], int]) token2IdFunc:
    :return: token Ids of the sentence
    """
    ids = []
    for token in sentence:
        try:
            i = token2IdFunc(token)
        except KeyError:
            # if character is not found in vocab, then mark it [UNK]
            i = token2IdFunc(unkToken)
        ids.append(i)
    return ids


def countLinesInFile(filepath: str):
    lines = 0
    with open(filepath, 'r') as f:
        for _ in f:
            lines += 1
    return lines
