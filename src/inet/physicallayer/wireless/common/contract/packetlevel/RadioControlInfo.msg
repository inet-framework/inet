//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Units;
import inet.physicallayer.wireless.common.contract.packetlevel.IModulation;

namespace inet::physicallayer;

//
// Message kinds for sending commands to the radio.
//
enum RadioCommandCode
{
    RADIO_C_CONFIGURE = 1;
}

//
// Control info attached to a configure command that is sent to the ~Radio.
//
class ConfigureRadioCommand extends cObject
{
    int radioMode = -1;                  // new radio mode or -1 if not set.
    W power = W(NaN);                    // new default transmission power in the range (0, +infinity) or NaN if not set.
    bps bitrate = bps(NaN);              // new default bitrate in the range (0, +infinity) or NaN if not set.
    IModulation *modulation;             // new default modulation or nullptr if not set.
    Hz centerFrequency = Hz(NaN);       // new default carrier frequency in the range (0, +infinity) or NaN if not set.
    Hz bandwidth = Hz(NaN);              // new default bandwidth in the rage (0, +infinity) or NaN if not set.
}
