//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.base.packetlevel;

import inet.common.LayeredProtocolBase;

//
// Module base for different physical layers.
//
simple PhysicalLayerBase extends LayeredProtocolBase
{
    parameters:
        @class(-);  //# set to '-' to prevent unintentional instantiation. Also disable the linter: suppress(missing-cpp-class)
        @display("i=block/wrxtx");
    gates:
        input upperLayerIn @labels(ILinkLayerFrame/down);
        output upperLayerOut @labels(ILinkLayerFrame/up);
        input radioIn @labels(IWirelessSignal);
}

