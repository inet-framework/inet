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

package inet.routing.leach;

import inet.routing.contract.IManetRouting;

simple Leach like IManetRouting
{
    parameters:
        @display("i=block/routing");
        string interfaceTableModule;   // The path to the InterfaceTable module
        double maxVariance = default(1); 
        int numNodes = default(10);
        double clusterHeadPercentage = default(0.5);
    gates:
        input ipIn;
        output ipOut;
}
