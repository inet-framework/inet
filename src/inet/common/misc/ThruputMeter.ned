//
// Copyright (C) 2005 Andras Varga
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

package inet.common.misc;

import inet.common.IHook;


//
// Thruput measurement utility module.
//
simple ThruputMeter like IHook
{
    parameters:
        @networkNode();
        double startTime @unit("s") = default(0s);
        int batchSize = default(50);
        double maxInterval @unit("s") = default(1s);
        @display("i=block/timer_vs");
    gates:
        input in;
        output out;
}


