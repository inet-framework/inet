//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.mobility.single;

import inet.mobility.base.MovingMobilityBase;

//
// Uses the <position_change> elements of the ANSim tool's
// trace file.
//
simple AnsimMobility extends MovingMobilityBase
{
    parameters:
        xml ansimTrace; // the ANSim trace file in XML
        int nodeId; // <position_change> elements to match; -1 gets substituted to parent module's index
        @class(AnsimMobility);
}

