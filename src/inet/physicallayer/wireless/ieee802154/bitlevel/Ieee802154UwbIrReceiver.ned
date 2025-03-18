//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.ieee802154.bitlevel;

import inet.common.Module;
import inet.physicallayer.wireless.common.contract.packetlevel.IReceiver;

//
// Receiver model for IEEE 802.15.4a Ultra-Wideband Impulse Radio (UWB-IR)
// communications. Implements energy detection techniques to receive and decode
// UWB-IR signals by integrating energy in specific time windows. Processes
// signals using dimensional representation, evaluates signal quality, and makes
// reception decisions based on computed SNIR. Designed for simulating low-power,
// short-range wireless sensor networks using the UWB-IR physical layer as
// specified in the IEEE 802.15.4a standard.
//
module Ieee802154UwbIrReceiver extends Module like IReceiver
{
    parameters:
        @class(Ieee802154UwbIrReceiver);
}

