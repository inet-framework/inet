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
class PcpTagBase extends TagBase
{
    int pcp = -1;
}

//
// This request determines the PCP that should be used to send the packet.
// It may be present on a packet from the application to the mac protocol.
//
class PcpReq extends PcpTagBase
{
}

//
// This indication specifies the PCP that was used to receive the packet.
// It may be present on a packet from the mac protocol to the application.
//
class PcpInd extends PcpTagBase
{
}
