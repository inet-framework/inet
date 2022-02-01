//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.propagation;

import inet.physicallayer.wireless.common.base.packetlevel.PropagationBase;

//
// This propagation model computes the propagation time to be proportional to
// the traveled distance, the ratio is determined by the constant propagation
// speed parameter.
//
module ConstantSpeedPropagation extends PropagationBase
{
    parameters:
        bool ignoreMovementDuringTransmission = default(true); // true means that the movement of the transmitter and the receiver during the signal transmission is ignored
        bool ignoreMovementDuringPropagation = default(true);  // true means that the movement of the transmitter and the receiver during the signal propagation is ignored
        bool ignoreMovementDuringReception = default(true);    // true means that the movement of the transmitter and the receiver during the signal reception is ignored
        @class(ConstantSpeedPropagation);
}

