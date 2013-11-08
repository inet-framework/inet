//
// Copyright (C) 2011 Juan Luis Garrote Molinero
// Copyright (C) 2013 Zsolt Prontvai
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

package inet.linklayer.ieee8021d.rstp;
import inet.linklayer.ieee8021d.ISpanningTree;

//
// Implements the Rapid Spanning Tree Protocol for IEEE 802 6-byte MAC addresses.
// Sends the hello message and configures the Port State of each Bridge Port,
// selecting some Bridge Ports to forward frames and others to discard frames,
// to provide a fully (spanning) and simply (tree) connected active topology.
//
simple RSTP like ISpanningTree
{
    parameters:
        double helloTime @unit("s") = default(2s);              // number of seconds RSTP waits between hello messages
        double fwdDelay @unit("s") = default(6s);               // upgrade delay
        double migrateTime @unit ("s")= default(30s);           // after that a not assigned port becomes designated
        double tcWhileTime @unit("s") = default(6s);            // a port will send TC=true during this time
        double maxAge @unit("s") = default(20s);                // max. hop count
        int priority = default(128);                            // priority + MAC determines the RSTP id
        bool autoEdge = default(true);                          // if false, edge ports will have to wait to get designated
        bool treeColoring = default(false);                     // shows RSTP tree
        string interfaceTableName = default("interfaceTable");  // The absolute path to the InterfaceTable
        string macTableName = default("macTable");  // The absolute path to the macTable
        @display("i=block/network2");
    gates:
        inout STPGate @labels(BPDU);                        // to the relay
}
