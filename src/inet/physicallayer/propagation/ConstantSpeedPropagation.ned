//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.physicallayer.propagation;

import inet.physicallayer.base.packetlevel.PropagationBase;

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
