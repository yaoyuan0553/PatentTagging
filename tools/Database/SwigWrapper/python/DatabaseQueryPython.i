%define DOCSTRING
"This is a SWIG-generated python wrapper for C++ database query \n"
"functionality, including APIs to extract various info from \n"
"generated database binary files by DatabaseGenerator"
%enddef

%include "std_string.i"
%include "std_vector.i"
%include "std_unordered_map.i"
%include "std_shared_ptr.i"
%include "carrays.i"
%include "cpointer.i"
%include "typemaps.i"


%shared_ptr(IdDataRecord);
%shared_ptr(DataRecordV2);

/* make IndexValue a read-only class */
%immutable IndexValue::pid;
%immutable IndexValue::aid;
%immutable IndexValue::appDate;
%immutable IndexValue::ipc;
%immutable IndexValue::binId;
%immutable IndexValue::ti;
%immutable IndexValue::ai;
%immutable IndexValue::ci;
%immutable IndexValue::di;
%immutable IndexValue::offset;


%module(docstring=DOCSTRING) DatabaseQuerySelectorPython
%{
#include "DatabaseQueryWrapper.h"
#include "DataBasicTypes.h"
%}


%apply unsigned int { uint32_t };
%apply unsigned long { uint64_t };

%feature("autodoc", "Python wrapper class representing a C++ std::string*") StringPtr;
%pointer_class(std::string, StringPtr)

%pointer_class(bool, boolPtr);

%array_class(int, IntArray);

namespace std {

    %template(StringVector) vector<string>;

    %template(IndexValueVector) vector<IndexValue*>;

    %template(IdDataRecordVector) vector<shared_ptr<IdDataRecord>>;

    %template(IdDataPartVector) vector<IdDataPart>;

    %template(UnorderedMapStringDataRecordV2) unordered_map<string, shared_ptr<DataRecordV2>>;
};

%include "DatabaseQueryWrapper.h"
%include "DataBasicTypes.h"

%template(PairII) Pair<int, int>;

