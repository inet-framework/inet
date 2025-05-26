//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.bitlevel;

//
// Module interface for pulse filters in wireless communication systems. Pulse
// filters convert between the sample domain and the symbol domain in the
// receiver's signal processing chain. They take a reception sample model as
// input and produce a reception symbol model as output by extracting symbols
// from the received signal samples based on the configured samples per symbol
// ratio.
//
// @see IPulseShaper
//
moduleinterface IPulseFilter
{
    parameters:
        @display("i=block/opticaldelay");
}

