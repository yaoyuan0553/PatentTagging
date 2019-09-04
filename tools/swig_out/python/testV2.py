"""
Sample program demonstrating how to use
this python wrapper of DatabaseQuery
for version 2 of DatabaseQueryManager
"""

import DatabaseQueryPython as dq
import tqdm
import random

dqm = dq.DatabaseQueryManagerV2("/media/yuan/Samsung_T5/patent_data/test/index.tsv",
                                "/media/yuan/Samsung_T5/patent_data/test/data")

pidList = dq.StringVector()
aidList = dq.StringVector()

dqm.getAllId(pidList, aidList)

print("\ngetAllId returned")
print(pidList.size(), aidList.size())

miniPidList = dq.StringVector([pidList[0], pidList[1], pidList[2]])

print(miniPidList.size())


print("requesting %s" % pidList[0])
info0 = dqm.getInfoById(pidList[0])

print(info0.stringify())

print("\ntesting speed of getContentById (unshuffled)")
# random.shuffle(pidList)
for pid in tqdm.tqdm(pidList):
    dr = dq.DataRecordV2()
    dqm.getContentById(pid, dr)
