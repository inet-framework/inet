//
// Copyright (C) 2016 Kai Kientopf <kai.kientopf@uni-muenster.de>
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
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.mobility.static;

import inet.mobility.base.MobilityBase;

//
// Places all hosts on a circle with radius based on the distance of the hosts.
//
simple StaticCircleMobility extends MobilityBase
{
    parameters:
        int numHosts;
        double initialZ @unit(m) = default(0m);
        double distance @unit(m) = default(100m);
        bool clockwise = default(true);
        int shift = default(0);

        @class(StaticCircleMobility);
}
