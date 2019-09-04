"""
Sample program demonstrating how to use
this python wrapper of DatabaseQuery
"""

import DatabaseQueryPython as dq
import tqdm


dqm = dq.DatabaseQueryManager("/media/yuan/Samsung_T5/patent_data/test/index.tsv",
                              "/media/yuan/Samsung_T5/patent_data/test/data")

pidList = dq.StringVector()
aidList = dq.StringVector()

dqm.getAllId(pidList, aidList)

print("\ngetAllId returned")
print(pidList.size(), aidList.size())

miniPidList = dq.StringVector([pidList[0], pidList[1], pidList[2]])

print(miniPidList.size())

print("testing getContentById speed")
with tqdm.tqdm(total=pidList.size()) as t:
    for pid in pidList:
        dr = dq.DataRecord()
        dqm.getContentById(pid, dr)
        t.update(1)

idDrList = dq.IdDataRecordVector()

dqm.getContentByIdList(miniPidList, idDrList)


print("\ngetContentByIdList returned")
for idDr in idDrList:
    print(idDr.pid, idDr.aid, idDr.dataRecord.title)
    x = idDr.dataRecord.title
    print("x before: %s" % x)
    x = "adfkasdf"
    print("x after: %s" % x)
    print(idDr.pid, idDr.aid, idDr.dataRecord.title)

idDataRecord = dq.IdDataRecord()

idDataRecord.aid = "safasd"
dr = dq.DataRecord(123, 5, 3, 3, 2, "title", "abstract", "claim", "description")
print(dr.stringify())
print(dr.stringify())
print(idDataRecord.dataRecord.stringify())


indexValue = dqm.getInfoById(pidList[0])

print("\ngetInfoById returned")
print(indexValue.stringify())

# fails as IndexValue is a read-only class
# indexValue.di = 2342

iv2 = dqm.getInfoById(pidList[0])


print("\nafter modify")
print("indexValue = %s" % indexValue.stringify())

print("iv2 = %s" % iv2.stringify())

# `StringPtr` is equivalent to C++'s std::string*
# you can get the python `str` value with .value() method
title = dq.StringPtr()
abstract = dq.StringPtr()
claim = dq.StringPtr()
description = dq.StringPtr()

# get title
dqm.getContentPartById(pidList[0], dqm.TITLE, title)
# get abstract
dqm.getContentPartById(pidList[0], dqm.ABSTRACT, abstract)
# get claim
dqm.getContentPartById(pidList[0], dqm.CLAIM, claim)

print(title.value())
print(abstract.value())
print(claim.value())

# output storage to store a list of title's
idTitleList = dq.IdDataPartVector()

# request DatabaseQueryManager to get title's from the corresponding list of PIDs
dqm.getContentPartByIdList(miniPidList, dqm.TITLE, idTitleList)

# print titles along with their PIDs
for idTitle in idTitleList:
    print("%s: %s" % (idTitle.pid, idTitle.dataPart))

# IdDataRecord* allocated by new does get correctly freed
# sample usage of a fast batch querying program for saving
# memory while keeping performance

with tqdm.tqdm(total=pidList.size()) as t:
    batch = 1024 * 32
    i = batch
    while i < pidList.size():
        idDrList = dq.IdDataRecordVector()
        dqm.getContentByIdList(pidList[i-batch:i], idDrList)
        i += batch
        t.update(batch)
