//
// ObstacleControl - models obstacles that block radio transmissions
// Copyright (C) 2010 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

package inet.environment;

module PhysicalEnvironment
{
    parameters:
        double temperature @unit(K) = default(293K);
        double pressure @unit(Pa) = default(101325Pa);
        double relativeHumidity = default(60);
        double spaceMinX @unit(m) = default(-1m/0); // min x position of space, unconstrained by default (negative infinity)
        double spaceMinY @unit(m) = default(-1m/0); // min y position of space, unconstrained by default (negative infinity)
        double spaceMinZ @unit(m) = default(-1m/0); // min z position of space, unconstrained by default (negative infinity)
        double spaceMaxX @unit(m) = default(1m/0); // max x position of space, unconstrained by default (positive infinity)
        double spaceMaxY @unit(m) = default(1m/0); // max y position of space, unconstrained by default (positive infinity)
        double spaceMaxZ @unit(m) = default(1m/0); // max z position of space, unconstrained by default (positive infinity)
        @display("i=misc/town_s");
        @class(PhysicalEnvironment);
}

