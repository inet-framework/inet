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
//        string vidSet = default("");
        int minVid = default(1);
        int maxVid = default(1);
        	// minVid and maxVid specify the range of allowed VID values for a port as follows:
        	// - tagged port: a range of VLAN Identifiers (VIDs) this port belongs to
        	// - untagged port: PVID for this port (should be a single VID, i.e., minVid = maxVid)
        	// - untagged port with dynamic tagging: a range of PVIDs for this port
    gates:
        inout macg;	// interface with Ethernet MAC
        inout relayg;	// interface with relay unit
}
