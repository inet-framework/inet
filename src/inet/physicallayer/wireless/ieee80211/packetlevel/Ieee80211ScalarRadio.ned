//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.ieee80211.packetlevel;

//
// Radio model for IEEE 802.11 networks that uses scalar representation for
// signal modeling. Represents signal power and attenuation as single scalar
// values, providing a balance between simulation accuracy and computational
// efficiency. Supports all IEEE 802.11 features of the base radio model while
// using a simpler signal representation than the dimensional model. Suitable for
// medium to large-scale network simulations where detailed frequency-domain
// analysis is not required.
//
module Ieee80211ScalarRadio extends Ieee80211Radio
{
    parameters:
        signalAnalogRepresentation = default("scalar");
}
