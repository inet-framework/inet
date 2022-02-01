//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.packetlevel;

//
// This receiver model receives an IEEE 802.11 transmission successfully if the
// minimum of the signal to noise and interference ratio over the duration of
// the reception is sufficiently high. It uses one of the IEEE 802.11 specific
// error models to compute the error rate based on this value and the used
// operation mode.
//
// @see ~Ieee80211DimensionalTransmitter, ~Ieee80211DimensionalRadio,
// ~Ieee80211DimensionalRadioMedium.
//
module Ieee80211DimensionalReceiver extends Ieee80211ReceiverBase
{
    parameters:
        @class(Ieee80211DimensionalReceiver);
}

