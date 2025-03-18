//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.ieee802154.bitlevel;

import inet.common.Module;
import inet.physicallayer.wireless.common.contract.packetlevel.ITransmitter;

//
// Generates pulse-level representation of IEEE 802.15.4A Ultra-Wideband Impulse
// Radio (UWB-IR) PHY frames. Implements the mandatory high PRF (Pulse Repetition
// Frequency) mode of the standard. Creates transmissions with appropriate signal
// characteristics by generating sync preambles, SFD (Start Frame Delimiter), and
// PHY headers with specific timing and energy parameters. Used for simulating
// the transmission side of IEEE 802.15.4A UWB-IR communication in low-power,
// short-range wireless networks.
//
module Ieee802154UwbIrTransmitter extends Module like ITransmitter
{
    parameters:
        @class(Ieee802154UwbIrTransmitter);
}

