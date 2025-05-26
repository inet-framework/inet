//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.bitlevel;

//
// Interface for Forward Error Correction (FEC) coders. Defines a common interface
// for modules that implement channel coding techniques to detect and correct
// transmission errors. FEC coders are used in bit-level physical layer
// simulations in wireless simulations to accurately model error correction
// capabilities of wireless protocols.
//
moduleinterface IFecCoder
{
    parameters:
        @display("i=block/segm");
}

