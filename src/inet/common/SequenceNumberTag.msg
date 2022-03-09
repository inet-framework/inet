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
class SequenceNumberTagBase extends TagBase
{
    int sequenceNumber = -1;
}

//
// This request determines the sequence number that should be used to send the packet.
// It may be present on a packet from the application to the mac protocol.
//
class SequenceNumberReq extends SequenceNumberTagBase
{
}

//
// This indication specifies the sequence number that was used to receive the packet.
// It may be present on a packet from the mac protocol to the application.
//
class SequenceNumberInd extends SequenceNumberTagBase
{
}
