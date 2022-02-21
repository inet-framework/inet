//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

//
// This is an abstract base class that should not be directly added as a tag.
//
class TosTagBase extends TagBase
{
    int tos = -1; // differentiated services code point
}

//
// This request determines the type of services that should be used to transmit the packet.
// It may be present on a packet from the application to the network protocol.
//
class TosReq extends TosTagBase
{
}

//
// This indication specifies the type of services that was used to receive the packet.
// It may be present on a packet from the network protocol to the application.
//
class TosInd extends TosTagBase
{
}
