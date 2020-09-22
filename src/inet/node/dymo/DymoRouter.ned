//
// Copyright (C) 2020 OpenSim Ltd.
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

package inet.node.dymo;

import inet.node.inet.AdhocHost;
import inet.routing.dymo.Dymo;

module DymoRouter extends AdhocHost
{
    submodules:
        dymo: Dymo {
            @display("p=825,226");
        }
    connections:
        dymo.ipOut --> tn.in++;
        dymo.ipIn <-- tn.out++;
}

