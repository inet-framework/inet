//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.ieee80211.llc;

import inet.linklayer.ieee8022.Ieee8022Llc;

//
// IEEE 802.11 Logical Link Control (LLC) implementation that uses LLC Protocol 
// Discrimination (LPD) for identifying upper layer protocols. Encapsulates packets 
// with standard IEEE 802.2 LLC headers, extending the Ieee8022Llc functionality 
// for use in 802.11 networks. Supports both basic LLC headers and SNAP headers 
// for protocol identification.
//
// @see ~IIeee80211Llc, ~Ieee80211LlcEpd, ~Ieee8022Llc, ~Ieee8022LlcHeader, ~Ieee8022LlcSnapHeader
//
simple Ieee80211LlcLpd extends Ieee8022Llc like IIeee80211Llc
{
    @class(Ieee80211LlcLpd);
}
