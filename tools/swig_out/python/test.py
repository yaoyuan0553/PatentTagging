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

# dr = dqm.getContentById

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

# IdDataRecord* allocated by new does get correctly freed
# sample usage of a fast batch querying program for saving
# memory while keeping performance
with tqdm.tqdm(total=pidList.size()) as t:
    batch = 1024 * 32
    i = 0
    while i < pidList.size():
        idDrList = dq.IdDataRecordVector()
        dqm.getContentByIdList(pidList[i:i+batch], idDrList)
        i += batch
        t.update(batch)
