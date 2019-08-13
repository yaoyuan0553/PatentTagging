//
// Created by yuan on 8/13/19.
//

#pragma once
#ifndef TOOLS_OUTPUTFORMATTERS_H
#define TOOLS_OUTPUTFORMATTERS_H

#include <vector>
#include <string>

#include <Utility.h>
#include <FunctorDict.h>

struct DatabaseOutputFormatter : public Cloneable {
    virtual std::string operator()(const std::vector<std::string>& texts) = 0;
    DECLARE_ABSTRACT_CLONE(DatabaseOutputFormatter);
};





using DatabaseOutputFormatterDict = FunctorDict<DatabaseOutputFormatter, std::string, const std::vector<std::string>&>;


#endif //TOOLS_OUTPUTFORMATTERS_H
