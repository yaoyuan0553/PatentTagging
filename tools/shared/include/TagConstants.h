//
// Created by yuan on 8/1/19.
//

#pragma once
#ifndef TOOLS_TAGCONSTANTS_H
#define TOOLS_TAGCONSTANTS_H

#include <string>


namespace tags
{
    constexpr char abstract[] = "abstract";
    constexpr char classification[] = "classification-ipcr";
    constexpr char content[] = "content";

    // special tag used to store file name of the current xml file
    constexpr char filename[] = "filename";

    constexpr char invention_title[] = "invention-title";
    constexpr char description[] = "description";
    constexpr char claims[] = "claims";
    constexpr char publication_reference[] = "publication-reference";
    constexpr char application_reference[] = "application-reference";
    constexpr char date[] = "date";
}


#endif //TOOLS_TAGCONSTANTS_H
