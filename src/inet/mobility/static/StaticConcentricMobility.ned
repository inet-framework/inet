//
// Copyright (C) 2014 Florian Meier <florian.meier@tu-harburg.de>
// Copyright (C) 2006 Isabel Dietrich <isabel.dietrich@informatik.uni-erlangen.de>
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

package inet.mobility.static;

import inet.mobility.base.MobilityBase;

//
// Places all hosts on concentric circles
//
//# TODO: rename to StaticConcentricCircleMobility?
simple StaticConcentricMobility extends MobilityBase
{
    parameters:
        int numHosts;
        double initialZ @unit(m) = default(0m);
        double distance @unit(m) = default(100m);

        @class(StaticConcentricMobility);
}

