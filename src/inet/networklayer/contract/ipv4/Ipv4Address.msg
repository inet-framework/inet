//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

cplusplus {{
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
}}

namespace inet;

cplusplus {{
typedef std::vector<Ipv4Address> Ipv4AddressVector;
}}

class Ipv4Address
{
    @existingClass;
    @opaque;
    @toString(.str());
    @fromString(Ipv4Address($));
}

class Ipv4AddressVector
{
   @existingClass;
   //FIXME tostring, ...
}
