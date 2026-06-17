//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

cplusplus {{
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"
}}

namespace inet;

cplusplus {{
typedef std::vector<Ipv6Address> Ipv6AddressVector;
}}

class Ipv6Address
{
    @existingClass;
    @opaque;
    @toString(.str());
}

class Ipv6AddressVector
{
    @existingClass;
}
