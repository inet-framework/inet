//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

package inet.linklayer.ieee8021as;

import inet.applications.contract.IApp;
import inet.node.inet.StandardHost;

//
// Provides a base module for IEEE 802.1AS gPTP end stations, extending the standard
// host with time synchronization capabilities. This module serves as the foundation
// for both master and slave nodes in a gPTP network.
//
// The module integrates:
// - A settable clock module that can be adjusted by the gPTP protocol
// - A gPTP protocol implementation for time synchronization
// - Standard networking capabilities inherited from StandardHost
//
// The clock module is configured to be settable by default, allowing the gPTP
// protocol to adjust its time based on synchronization messages. This is essential
// for maintaining precise time synchronization across the network.
//
// This module is typically not used directly but serves as a base class for more
// specific implementations such as ~GptpMaster and ~GptpSlave, which configure
// the gPTP protocol for their specific roles in the time synchronization hierarchy.
//
// @see ~GptpMaster, ~GptpSlave, ~Gptp
//
module GptpEndstation extends StandardHost
{
    parameters:
        clock.typename = default("SettableClock");
    submodules:
        gptp: <default("Gptp")> like IApp if typename != "" {
            @display("p=700.11755,75.166");
        }
    connections:
        gptp.socketOut --> at.in++ if exists(gptp);
        at.out++ --> gptp.socketIn if exists(gptp);
}
