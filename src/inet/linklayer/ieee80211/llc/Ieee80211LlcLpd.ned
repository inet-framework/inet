//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.ieee80211.llc;

import inet.linklayer.ieee8022.Ieee8022Llc;

//
// An LLC implementation that encapsulates packets in an LLC header,
// using IEEE 802 LPD-style encoding.
//
// See ~Ieee8022LlcHeader, Ieee8022LlcSnapHeader.
//
simple Ieee80211LlcLpd extends Ieee8022Llc like IIeee80211Llc
{
    @class(Ieee80211LlcLpd);
}
