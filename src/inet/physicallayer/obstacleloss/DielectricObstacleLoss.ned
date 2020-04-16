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

package inet.physicallayer.obstacleloss;

import inet.physicallayer.base.packetlevel.TracingObstacleLossBase;

//
// This obstacle loss model determines the power loss by computing the accurate
// dielectric and reflection loss along the straight path considering the shape,
// the position, the orientation, and the material of obstructing physical
// objects.
//
module DielectricObstacleLoss extends TracingObstacleLossBase
{
    parameters:
        bool enableDielectricLoss = default(true); // enables/disables material dielectric loss
        bool enableReflectionLoss = default(true); // enables/disables surface reflection loss 
        @class(DielectricObstacleLoss);
}

