//
// Created by yuan on 9/5/19.
//

#include <tqdm.h>
#include <chrono>

#include "DatabaseQueryWrapper.h"

using namespace std;

void test()
{
    DatabaseQueryManagerV2 dqm("/media/yuan/Samsung_T5/patent_data/test/index.tsv",
                               "/media/yuan/Samsung_T5/patent_data/test/data");
//    DatabaseQueryManagerV2 dqm("/media/yuan/raid0_disk1/test/index.tsv",
//            "/media/yuan/raid0_disk1/test/data");

    vector<string> pidList, aidList;
    dqm.getAllId(&pidList, &aidList);

//    tqdm bar;
//    int i = 0;
//    for (const string& pid : pidList) {
//        DataRecordV2 dr;
//        dqm.getContentById(pid.c_str(), &dr);
//        bar.progress(++i, pidList.size());
//    }
//    bar.finish();
    vector<string> miniPidList(pidList.begin(), pidList.begin() + 2000);
    unordered_map<string, shared_ptr<DataRecordV2>> dataRecordById;
    for (const string &pid: pidList)
        dataRecordById.emplace(pid, new DataRecordV2);

    auto begin = chrono::high_resolution_clock::now();
    dqm.getContentByPidList(dataRecordById);
    auto totalTime = chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - begin);
    printf("getContentByPidList took %.6f s for %zu items, speed %.2f Hz\n",
           totalTime.count(), dataRecordById.size(), dataRecordById.size() / totalTime.count());
}

int main()
{
    test();

    return 0;
}
