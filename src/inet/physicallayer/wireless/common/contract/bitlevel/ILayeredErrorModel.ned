//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.common.contract.bitlevel;

//
// Interface for layered error models in wireless communications. Provides
// functionality for computing errors at different protocol layers (packet, bit,
// symbol, sample) based on signal-to-noise-and-interference ratio (SNIR).
// Enables simplified modeling of the physical layer without implementing all
// details, allowing for different levels of abstraction and complexity in
// wireless transmission error simulation.
//
moduleinterface ILayeredErrorModel
{
    parameters:
        @display("i=block/broadcast");
}
