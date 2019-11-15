//
// Copyright (C) 2006 Andras Varga
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

package inet.mobility.static;

import inet.mobility.base.MobilityBase;

//
// This mobility module does nothing; it can be used for stationary nodes.
//
// @author Andras Varga
//
simple StationaryMobility extends MobilityBase
{
    parameters:
        double initialX @unit(m) = default(uniform(this.constraintAreaMinX, this.constraintAreaMaxX));
        double initialY @unit(m) = default(uniform(this.constraintAreaMinY, this.constraintAreaMaxY));
        double initialZ @unit(m) = default(nanToZero(uniform(this.constraintAreaMinZ, this.constraintAreaMaxZ)));
        double initialLatitude @unit(deg) = default(nan deg);
        double initialLongitude @unit(deg) = default(nan deg);
        double initialAltitude @unit(m) = default(0m);
        double initialHeading @unit(deg) = default(0deg);
        double initialElevation @unit(deg) = default(0deg);
        double initialBank @unit(deg) = default(0deg);
        bool initFromDisplayString = default(true); // enables one time initialization from the subject module's display string
        bool updateFromDisplayString = default(true); // enables continuous update from the subject module's display string for dragging and rotating it
        @class(StationaryMobility);
}

