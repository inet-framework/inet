///
/// @file   VLANTagger.ned
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jan/5/2012
///
/// @brief  Defines VLAN tagger module for VLAN tagging based on IEEE 802.1Q.
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///

package inet.linklayer.vlan;

//
// TODO auto-generated type
//
simple VLANTagger
{
    parameters:
        @display("i=block/layer;p=92,124");
        bool tagged = default(false);	// untagged port is for access links, while tagged port is for trunk (inter-switch) links
        bool dynamicTagging = default(false);	// (experimental) dynamic tagging for untagged port based on source & destination MAC addresses of incoming frames
        string vidSet = default("");
        	// - tagged port: a set of VLAN Identifiers (VIDs) this port belongs to
        	// - untagged port: PVID for this port (should be a single VID)
        	// - untagged port with dynamic tagging: a set of PVIDs for this port
//        int pvid = default(1);	// Port VID (PVID)
        //string vidSet;	// set of VIDs for Port-and-Protocol-based VLAN classification (not supported yet)
        //bool verbose = default(false);	// module verbosity
        //int priority = default(5);
    gates:
        inout macg;	// interface with Ethernet MAC
        inout relayg;	// interface with relay unit
}
