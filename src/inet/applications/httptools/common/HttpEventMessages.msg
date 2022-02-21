//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
//
// SPDX-License-Identifier: GPL-3.0-or-later
//

import inet.common.INETDefs;

namespace inet::httptools;

//
// Server status update event
//
// Used by the controller to trigger popularity modifications for web sites.
//
// @author  Kristjan V. Jonsson
//
message HttpServerStatusUpdateMsg
{
    string www;             // Name of the WWW site.
    simtime_t setTime;      // The event set time.
    int eventKind;          // The event kind, e.g. ss_special.
    double pvalue;          // The initial elevated probability.
    double pamortize;       // The amortization factor -- i.e. how much the pvalue is decreased on each hit.
}
