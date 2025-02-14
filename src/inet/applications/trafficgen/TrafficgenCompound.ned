//
// Copyright (C) 2015 - 2019 Felix Weinrank
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.applications.trafficgen;

import inet.applications.contract.IApp;
import inet.applications.trafficgen.ITrafficgen;
import inet.applications.trafficgen.ITrafficgenHandler;

module TrafficgenCompound like IApp
{
    parameters:
        int numGenerators = default(0);
        @display("i=block/broadcast;bgb=353,249");

    gates:
        input socketIn @labels(SCTPControlInfo/up);
        output socketOut @labels(SCTPControlInfo/down);

    submodules:
        generator[numGenerators]: <> like ITrafficgen {
            @display("p=139,44");
        }

        handler: <> like ITrafficgenHandler {
            @display("p=139,152");
        }
    connections:
        for i=0..numGenerators-1 {
            generator[i].generatorOut --> handler.generatorIn++;
            generator[i].generatorIn <-- handler.generatorOut++;
        }

        handler.socketOut --> socketOut;
        handler.socketIn <-- socketIn;
}
