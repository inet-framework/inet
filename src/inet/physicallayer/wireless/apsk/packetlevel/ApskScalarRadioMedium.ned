//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.physicallayer.wireless.apsk.packetlevel;

import inet.physicallayer.wireless.common.medium.RadioMedium;

//
// This radio medium model provides a hypothetical radio that simply uses one of
// the  well-known modulations without utilizing other techiques such as forward
// error correction, interleaving, spreading, etc. It must be used in
// conjunction with the ~ApskScalarRadio model.
//
// @see ~ApskScalarRadio, ~ScalarAnalogModel, ~ApskScalarTransmitter,
// ~ApskScalarReceiver.
//
module ApskScalarRadioMedium extends RadioMedium
{
    parameters:
        analogModel.typename = default("ScalarAnalogModel");
        backgroundNoise.typename = default("IsotropicScalarBackgroundNoise");
}

