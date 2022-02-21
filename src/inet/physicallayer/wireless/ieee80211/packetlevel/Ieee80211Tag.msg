//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.physicallayer.wireless.ieee80211.mode.IIeee80211Mode;
import inet.physicallayer.wireless.ieee80211.mode.Ieee80211Channel;

namespace inet::physicallayer;

//
// This is an abstract base class that should not be directly added as a tag.
//
class Ieee80211ModeTagBase extends TagBase
{
    const IIeee80211Mode *mode; // IEEE 802.11 transmission mode
}

//
// This request determines the IEEE 802.11 mode that should be used to transmit the packet.
// It may be present on a packet from the application to the phyisical layer.
//
class Ieee80211ModeReq extends Ieee80211ModeTagBase
{
}

//
// This indication specifies the IEEE 802.11 mode that was used to receive the packet.
// It may be present on a packet from the phyiscal layer to the application.
//
class Ieee80211ModeInd extends Ieee80211ModeTagBase
{
}

//
// This is an abstract base class that should not be directly added as a tag.
//
class Ieee80211ChannelTagBase extends TagBase
{
    const Ieee80211Channel *channel; // IEEE 802.11 band and channel
}

//
// This request determines the IEEE 802.11 channel that should be used to transmit the packet.
// It may be present on a packet from the application to the phyisical layer.
//
class Ieee80211ChannelReq extends Ieee80211ChannelTagBase
{
}

//
// This indication specifies the IEEE 802.11 channel that was used to receive the packet.
// It may be present on a packet from the phyiscal layer to the application.
//
class Ieee80211ChannelInd extends Ieee80211ChannelTagBase
{
}
