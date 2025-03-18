//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.ieee802154.bitlevel;

import inet.physicallayer.wireless.common.radio.packetlevel.Radio;

//
// Implements the IEEE 802.15.4A Ultra-Wideband Impulse Radio (UWB-IR) physical
// layer. Provides a complete radio implementation with specialized transmitter
// and receiver components for UWB-IR communication. Supports pulse-level
// representation of IEEE 802.15.4A UWB PHY frames using the mandatory high PRF
// (Pulse Repetition Frequency) mode. Used for simulating low-power, short-range
// wireless communication in applications like wireless sensor networks and IoT
// devices.
//
module Ieee802154UwbIrRadio extends Radio
{
    parameters:
        transmitter.typename = default("Ieee802154UwbIrTransmitter");
        receiver.typename = default("Ieee802154UwbIrReceiver");
        @class(Radio);
}
