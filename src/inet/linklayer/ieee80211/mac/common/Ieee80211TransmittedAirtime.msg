//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


import inet.common.INETDefs;
import inet.linklayer.common.MacAddress;

namespace inet::ieee80211;

//
// Carries the on-air time actually consumed by a transmitted frame, together with
// the receiver it was sent to. Emitted by the coordination function (~Dcf, ~Hcf)
// as the object value of the `frameTransmittedAirtime` signal, once per on-air
// transmission (so retries are counted), and consumed by ~AirtimeFairnessQueue to
// charge the receiver's airtime deficit a-posteriori.
//
class Ieee80211TransmittedAirtime extends cObject
{
    MacAddress receiverAddress;
    simtime_t airtime;
}
