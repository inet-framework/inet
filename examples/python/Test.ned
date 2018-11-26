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

package inet.examples.python;

import inet.common.PythonScripting;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;


network Test
{
    @display("bgb=550,308");
    submodules:
        pythonScripting: PythonScripting {
            @display("p=187,199");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=88,46");
        }
        host: StandardHost {
            @display("p=312,99");
        }
        standardHost: StandardHost {
            @display("p=422,181");
        }
    connections:
        host.ethg++ <--> Eth100M <--> standardHost.ethg++;
}
