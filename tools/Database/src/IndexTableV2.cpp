//
// Created by yuan on 9/2/19.
//

#include "IndexTableV2.h"

#include <string>
#include <fstream>
#include <sstream>

using namespace std;

ostream &operator<<(std::ostream& os, const IndexValueList& ivl)
{
    for (const IndexValue* iv : ivl)
        os << *iv << '\n';

    return os;
}

istream& operator>>(istream& is, IndexValueList& ivl)
{
    for (string line; getline(is, line);) {
        istringstream ss(line);
        auto* iv = new IndexValue;
        ss >> *iv;

        ivl.push_back(iv);
    }

    return is;
}


IndexTableV2::IndexTableV2(const char* filename, bool hasHeader)
{
    ifstream ifs(filename);
    if (!ifs.is_open()) {
        fprintf(stderr, "file [%s] failed to open\n", filename);
        PERROR("ifstream open");
    }

    if (hasHeader) {
        string line;
        getline(ifs, line); // discard the first line as it is a title
    }
    ifs >> indexValueList_;

    for (const IndexValue* iv : indexValueList_) {
        binId2Index_[iv->binId].push_back(iv);
        pid2Index_[iv->pid] = iv;
        aid2Index_[iv->aid] = iv;
    }

    ifs.close();
}

IndexTableV2::~IndexTableV2()
{
    for (const IndexValue* iv : indexValueList_)
        delete iv;
}
