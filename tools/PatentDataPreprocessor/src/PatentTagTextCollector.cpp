//
// Created by yuan on 7/30/19.
//

#include "PatentTagTextCollector.h"

#include <filesystem>

#include <pugixml.hpp>



namespace fs = std::filesystem;
using namespace std;

const initializer_list<char32_t> PatentTagTextCollector::defaultSeparators_ =
        { u'。' };


void PatentTagTextCollector::internalRun()
{
    for (;;)
    {
        pugi::xml_document doc;
        auto [filename, quit] = filenameQueue_.pop();

        if (quit) break;

        pugi::xml_parse_result result = doc.load_file(filename.c_str());
        if (!result)
            continue;

        walker_.reset();

        try {
            doc.traverse(walker_);
        }
        catch (std::range_error& e) {
            std::cerr << e.what() << '\n';
            std::cerr << "[" << filename << "]\n";
            continue;
        }
    }
}


void PatentTagTextCollector::processTagTexts(const TagTextDict& tagTextDict)
{

}


