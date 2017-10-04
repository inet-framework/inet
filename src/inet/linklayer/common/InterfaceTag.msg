//
// This program is property of its copyright holder. All rights reserved.
// 

cplusplus{{
#include "inet/common/TagBase_m.h"
}}

namespace inet;

class TagBase extends cObject;

class InterfaceTagBase extends TagBase
{
    int interfaceId = -1; // a locally unique (network node) identifier that determines the interface registered in the ~InterfaceTable  
}

//
// Determines the outgoing interface where the packet should leave the network node.
//
class InterfaceReq extends InterfaceTagBase
{
}

//
// Specifies the incoming interface where the packet has arrived at the network node.
//
class InterfaceInd extends InterfaceTagBase
{
}

cplusplus{{
Register_Tag(InterfaceReq);
Register_Tag(InterfaceInd);
}}
