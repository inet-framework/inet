//
// This program is property of its copyright holder. All rights reserved.
// 

//
// TODO generated message class
//

cplusplus {{
#include "INETDefs.h"
}}

namespace inet;

class TLVOptionBase
{
    short type = -1;
    short length = 0;   // total length of option
}

class TLVOptionRaw extends TLVOptionBase
{
    char bytes[];
}

class TLVOptions
{
    @customize(true);
    abstract TLVOptionBase tlvOption[];
}

