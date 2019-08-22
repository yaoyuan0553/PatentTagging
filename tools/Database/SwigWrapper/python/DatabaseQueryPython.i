%module DatabaseQuerySelectorPython
%{
#include "DatabaseQueryWrapper.h"
%}

%include "std_string.i"
%include "carrays.i"
%include "DatabaseQueryWrapper.h"

%array_class(int, IntArray);

%template(PairII) Pair<int, int>;
