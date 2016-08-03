//
// This program is property of its copyright holder. All rights reserved.
// 

cplusplus{{
#include "inet/common/TagBase_m.h"
}}

namespace inet;

class TagBase extends cObject;

class InterfacePortTagBase extends TagBase
{
    int interfacePort = -1; // a locally unique (network node) identifier that determines the interface gate index on relay module
}

//
// Determines the outgoing interface where the packet should leave the network node.
//
class InterfacePortReq extends InterfacePortTagBase
{
}

//
// Specifies the incoming interface where the packet has arrived at the network node.
//
class InterfacePortInd extends InterfacePortTagBase
{
}
