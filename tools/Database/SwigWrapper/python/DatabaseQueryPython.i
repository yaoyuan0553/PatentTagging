%define DOCSTRING
"This is a SWIG-generated python wrapper for C++ database query \n"
"functionality, including APIs to extract various info from \n"
"generated database binary files by DatabaseGenerator"
%enddef

%module(py3=1, docstring=DOCSTRING, doxygen=1) DatabaseQuerySelectorPython
%{
#include "DatabaseQueryWrapper.h"
#include "DataBasicTypes.h"
%}

%include "std_string.i"
%include "std_vector.i"
%include "std_shared_ptr.i"
%include "carrays.i"
%include "cpointer.i"
%include "typemaps.i"

%feature("autodoc", "3");
%apply unsigned int { uint32_t };
%apply unsigned long { uint64_t };

//%apply bool* OUTPUT { bool* exist };
%pointer_class(bool, boolPtr);

%array_class(int, IntArray);

namespace std {
    %apply const string& {const string* title,
                               const string* abstract,
                               const string* claim,
                               const string* description};
    %template(StringArray) vector<string>;
    %template(IndexValuePtrArray) vector<IndexValue*>;
    %template(IdDataRecordSharedPtr) shared_ptr<IdDataRecord>;
    %shared_ptr(IdDataRecord)
    %template(IdDataRecordArray) vector<shared_ptr<IdDataRecord>>;
//    %template(IdDataRecordArray) vector<IdDataRecord*>;

    %apply IdDataRecord { shared_ptr<IdDataRecord>* };
    %apply IdDataRecord { IdDataRecordSharedPtr* };
};

%include "DatabaseQueryWrapper.h"
%include "DataBasicTypes.h"

%template(PairII) Pair<int, int>;


