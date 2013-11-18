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
// Implements the Rapid Spanning Tree Protocol (IEEE 802.D-2004) for IEC 48-bit
// MAC addresses. It is a complete implementation except it doesn't fall back
// to STP when peers don't support RSTP.
//
// RSTP is a network protocol that ensures a loop-free topology for any
// bridged Ethernet local area network. The basic function of RSTP
// is to prevent bridge loops and the broadcast radiation that results from them.
// Spanning tree also allows a network design to include spare (redundant) links to
// provide automatic backup paths if an active link fails, without the danger of
// bridge loops, or the need for manual enabling/disabling of these backup links.
// RSTP is an improved version of STP, providing faster convergence.
//
// RSTP makes use of per-port configuration data such as cost and priority.
// They can be set using ~L2NetworkConfigurator. Port state is kept in the switch's
// ~InterfaceTable.
//
// @see L2NetworkConfigurator, InterfaceTable
//
simple RSTP like ISpanningTree
{
    parameters:
        double helloTime @unit("s") = default(2s);              // interval between sending Hello messages
        double fwdDelay @unit("s") = default(6s);               // time a port is kept in the learning state before going into forwarding
        double migrateTime @unit ("s")= default(30s);           // time after which a non-assigned port becomes designated
        double tcWhileTime @unit("s") = default(6s);            // a port will send TC=true (Topology Change Notification) for this time interval
        double maxAge @unit("s") = default(20s);                // max. hop count
        int priority = default(128);                            // switch priority
        bool autoEdge = default(true);                          // if false, edge ports will have to wait to get designated
        bool treeColoring = default(false);                     // shows the RSTP tree by coloring connections in the network graphics
        string interfaceTableName = default("interfaceTable");  // module path to the InterfaceTable module
        string macTableName = default("macTable");              // module path to the MACAddressTable module
        @display("i=block/network2");
    gates:
        input relayIn;
        output relayOut;
}
