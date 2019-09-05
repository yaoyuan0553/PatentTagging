"""
Sample program demonstrating how to use
this python wrapper of DatabaseQuery
for version 2 of DatabaseQueryManager
"""

import DatabaseQueryPython as dq
import random
import time
from tqdm import tqdm

dqm = dq.DatabaseQueryManagerV2("/media/yuan/Samsung_T5/patent_data/test/index.tsv",
                                "/media/yuan/Samsung_T5/patent_data/test/data")

pidList = dq.StringVector()
aidList = dq.StringVector()

dqm.getAllId(pidList, aidList)

print("\ngetAllId returned")
print(pidList.size(), aidList.size())

miniPidList = dq.StringVector(pidList[:2000])

print(miniPidList.size())


print("requesting %s" % pidList[0])
info0 = dqm.getInfoById(pidList[0])

print(info0.stringify())


drv2 = dq.DataRecordV2()
dqm.getContentById(pidList[0], drv2)
# print(drv2.stringify())

dataRecordById = dq.UnorderedMapStringDataRecordV2()

for pid in pidList:
    dataRecordById[pid] = dq.DataRecordV2()

begin = time.time()
dqm.getContentByPidList(dataRecordById)
totalTime = time.time() - begin
print("getContentByPidList took %.6f s for %s items, speed %.2f Hz" %
      (totalTime, dataRecordById.size(), dataRecordById.size() / totalTime))

# for pid, dataRecord in dataRecordById.items():
#     print("%s: title: %s" % (pid, dataRecord.title))


# print("\ntesting speed of getContentById (unshuffled)")
# # random.shuffle(pidList)
# for pid in tqdm.tqdm(pidList):
#     dr = dq.DataRecordV2()
#     dqm.getContentById(pid, dr)
