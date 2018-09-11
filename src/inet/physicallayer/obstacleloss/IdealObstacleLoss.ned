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
// This obstacle loss model determines power loss by checking if there is any
// obstructing physical object along the straight propagation path. The result
// is either total power loss if there was such an object or no loss at all if
// there wasn't.
//
module IdealObstacleLoss extends TracingObstacleLossBase
{
    parameters:
        @class(IdealObstacleLoss);
}

