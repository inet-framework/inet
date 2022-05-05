//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

//
// This is an abstract base class that should not be directly added as a tag.
//
class DropEligibleTagBase extends TagBase
{
    bool DropEligible = false;
}

//
// This request determines the DropEligible that should be used to send the packet.
// It may be present on a packet from the application to the mac protocol.
//
class DropEligibleReq extends DropEligibleTagBase
{
}

//
// This indication specifies the DropEligible that was used to receive the packet.
// It may be present on a packet from the mac protocol to the application.
//
class DropEligibleInd extends DropEligibleTagBase
{
}
