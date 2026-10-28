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
// The C-tag and S-tag versions below are the tags of the C-tag (customer VLAN
// tag, TPID 0x8100) and of the S-tag (service VLAN tag, TPID 0x88A8, IEEE
// 802.1ad) of a frame. A node that inserts or removes both tags carries both
// versions at the same time.
//
class DropEligibleTagBase extends TagBase
{
    bool DropEligible = false;
}

//
// This request determines the drop eligible indicator (DEI) of the C-tag that should be used
// to send the packet. It may be present on a packet from the application to the
// mac protocol.
//
class CDropEligibleReq extends DropEligibleTagBase
{
}

//
// This indication specifies the drop eligible indicator (DEI) of the C-tag that was used to
// receive the packet. It may be present on a packet from the mac protocol to the
// application.
//
class CDropEligibleInd extends DropEligibleTagBase
{
}

//
// This request determines the drop eligible indicator (DEI) of the S-tag that should be used
// to send the packet. It may be present on a packet from the application to the
// mac protocol.
//
class SDropEligibleReq extends DropEligibleTagBase
{
}

//
// This indication specifies the drop eligible indicator (DEI) of the S-tag that was used to
// receive the packet. It may be present on a packet from the mac protocol to the
// application.
//
class SDropEligibleInd extends DropEligibleTagBase
{
}
