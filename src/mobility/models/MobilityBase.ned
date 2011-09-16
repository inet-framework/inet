//
// Copyright (C) 2005 Andras Varga
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


package inet.mobility.models;

import inet.mobility.IMobility;


//
// Abstract base module for mobility models.
//
// @author Andras Varga
//
simple MobilityBase like IMobility
{
    parameters:
        bool debug = default(false);
        double constraintAreaX @unit(m) = default(0m); // min x position of the constraint area
        double constraintAreaY @unit(m) = default(0m); // min y position of the constraint area
        double constraintAreaZ @unit(m) = default(0m); // min z position of the constraint area
        double constraintAreaSizeX @unit(m) = default(-1m); // x size of the constraint area, -1 means unbounded
        double constraintAreaSizeY @unit(m) = default(-1m); // y size of the constraint area, -1 means unbounded
        double constraintAreaSizeZ @unit(m) = default(-1m); // z size of the constraint area, -1 means unbounded
}
