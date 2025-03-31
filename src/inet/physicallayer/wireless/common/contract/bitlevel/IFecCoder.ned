//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.common.contract.bitlevel;

//
// Interface for Forward Error Correction (FEC) coders in wireless communications.
// Defines a common interface for modules that implement channel coding techniques
// to detect and correct transmission errors. Supports various coding schemes like
// convolutional coding with different code rates and constraint lengths. Used in
// bit-level physical layer simulations to accurately model error correction
// capabilities of wireless protocols.
//
moduleinterface IFecCoder
{
    parameters:
        @display("i=block/segm");
}
