//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.ieee80211.llc;

import inet.linklayer.ieee802.Ieee802Epd;

//
// An LLC implementation that encapsulates packets with the IEEE 802 EtherType
// Protocol Discrimination (EPD) header, as defined in the section 9.2 EtherTypes
// of the IEEE Std 802-2014 standard. See ~Ieee802EpdHeader.
//
simple Ieee80211LlcEpd extends Ieee802Epd like IIeee80211Llc
{
    @class(Ieee80211LlcEpd);
}

