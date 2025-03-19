//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.shortcut;

import inet.physicallayer.wireless.common.base.packetlevel.PhysicalLayerBase;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadio;

//
// Implements a simple shortcut to peer radio protocol that completely
// bypasses the physical medium. This radio module directly sends packets to the
// other radio module without any physical layer processing in the radio medium.
// Packets received from the upper layer protocols may be lost. Physical layer
// overhead is simply simulated by physical header bits, preamble transmission
// duration, and a propagation delay.
//
simple ShortcutRadio extends PhysicalLayerBase like IRadio
{
    parameters:
        string interfaceTableModule;
        double bitrate @unit(bps);
        volatile int lengthOverhead @unit(b) = default(0b); // Extra length added to packets
        volatile double durationOverhead @unit(s) = default(0s); // Extra transmission duration added to packets
        volatile double propagationDelay @unit(s) = default(0s); // Propagation time to all destinations
        volatile double packetLoss = default(0); // Packet loss probability
        @class(ShortcutRadio);
        @signal[transmissionStateChanged](type=long);    // Type=inet::physicallayer::TransmissionState
}

