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

package inet.tutorials.visualization;

import inet.node.ethernet.Eth100M;
import inet.node.inet.Router;
import inet.node.wireless.AccessPoint;


network VisualizationF extends VisualizationE
{
    submodules:
        accessPoint1: AccessPoint {
            parameters:
                @display("p=601,163");
        }
        router1: Router {
            parameters:
                @display("p=601,234;i=abstract/router");
        }
    connections:
        router0.pppg++ <--> Eth100M <--> router1.pppg++;
        accessPoint1.ethg++ <--> Eth100M <--> router1.ethg++;
}
