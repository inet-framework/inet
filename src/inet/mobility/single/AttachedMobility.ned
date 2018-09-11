//
// Copyright (C) OpenSim Ltd.
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

package inet.mobility.single;

import inet.mobility.base.MobilityBase;

//
// This module provides a mobility that is attached to another mobility at a
// given offset. The position, velocity and acceleration are all affectred by
// the respective quantites and the orientation of the mobility where this one
// is attached.
//
simple AttachedMobility extends MobilityBase
{
    parameters:
        string mobilityModule;
        double offsetX @unit(m) = default(0m);
        double offsetY @unit(m) = default(0m);
        double offsetZ @unit(m) = default(0m);
        double offsetHeading @unit(deg) = default(0deg);
        double offsetElevation @unit(deg) = default(0deg);
        double offsetBank @unit(deg) = default(0deg);
        @class(AttachedMobility);
}

