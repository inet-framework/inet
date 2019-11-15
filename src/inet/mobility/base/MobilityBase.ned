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

package inet.mobility.base;

import inet.mobility.contract.IMobility;

//
// Abstract base module for mobility models.
//
// Format directives:
//  - %p position
//  - %v velocity
//  - %s speed
//  - %a acceleration
//  - %P angular position
//  - %V angular velocity
//  - %S angular speed
//  - %A angular acceleration
//
// @author Andras Varga
//
simple MobilityBase like IMobility
{
    parameters:
        @signal[mobilityStateChanged](type=inet::MobilityBase);     // it's work in inet, but not work in extended module in other namespace
        @display("i=block/cogwheel");
        string subjectModule = default("^"); // module path which determines the subject module, the motion of which this mobility model describes, the default value is the parent module
        string coordinateSystemModule = default(""); // module path of the geographic coordinate system module
        string displayStringTextFormat = default("p: %p\nv: %v"); // format string for the mobility module's display string text
        bool updateDisplayString = default(true); // enables continuous update of the subject module's position via modifying its display string
        double constraintAreaMinX @unit(m) = default(-inf m); // min x position of the constraint area, unconstrained by default (negative infinity)
        double constraintAreaMinY @unit(m) = default(-inf m); // min y position of the constraint area, unconstrained by default (negative infinity)
        double constraintAreaMinZ @unit(m) = default(-inf m); // min z position of the constraint area, unconstrained by default (negative infinity)
        double constraintAreaMaxX @unit(m) = default(inf m); // max x position of the constraint area, unconstrained by default (positive infinity)
        double constraintAreaMaxY @unit(m) = default(inf m); // max y position of the constraint area, unconstrained by default (positive infinity)
        double constraintAreaMaxZ @unit(m) = default(inf m); // max z position of the constraint area, unconstrained by default (positive infinity)
}

