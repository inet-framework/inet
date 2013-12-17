//
// Copyright (C) 2006 Andras Varga
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


//
// @page ieee80211.html, IEEE 802.11 Model
//
// This page provides an overview of the IEEE 802.11 model for the INET Framework.
//
// An IEEE 802.11 interface (NIC) comes in several flavours, differring
// in their role (ad-hoc station, infrastructure mode station, or
// access point) and their level of detail:
//
//  - ~Ieee80211Nic: a generic (configurable) NIC
//  - ~Ieee80211Nic with mgmtType = ~Ieee80211MgmtAdhoc: for ad-hoc mode
//  - ~Ieee80211Nic with mgmtType = ~Ieee80211MgmtAP or ~Ieee80211MgmtAPSimplified: for use in an access point
//  - ~Ieee80211Nic with mgmtType = ~Ieee80211MgmtSTA, ~Ieee80211MgmtSTASimplified: for use in an
//    infrastructure-mode station
//
// NICs consist of four layers, which are the following (in top-down order):
//
//  -# agent
//  -# management
//  -# MAC
//  -# physical layer (radio)
//
// <i>The physical layer</i> modules (~Ieee80211Radio) deal with modelling
// transmission and reception of frames. They model the characteristics of
// the radio channel, and determine if a frame was received correctly
// (that is, it did not suffer bit errors due to low signal power or
// interference in the radio channel). Frames received correctly are passed
// up to the MAC. The implementation of these modules is based on the
// Mobility Framework.
//
// <i>The MAC layer</i> (~Ieee80211Mac) performs transmission of frames according
// to the CSMA/CA protocol. It receives data and management frames from
// the upper layers, and transmits them.
//
// <i>The management layer</i> performs encapsulation and decapsulation of data packets
// for the MAC, and exchanges management frames via the MAC with its peer
// management entities in other STAs and APs. Beacon, Probe Request/Response,
// Authentication, Association Request/Response etc frames are generated
// and interpreted by management entities, and transmitted/received via
// the MAC layer. During scanning, it is the management entity that periodically
// switches channels, and collects information from received beacons and
// probe responses.
//
// The management layer has several implementations which differ in their role
// (STA/AP/ad-hoc) and level of detail: ~Ieee80211MgmtAdhoc,
// ~Ieee80211MgmtAP, ~Ieee80211MgmtAPSimplified, ~Ieee80211MgmtSTA,
// ~Ieee80211MgmtSTASimplified. The ..Simplified ones differ from the others
// in that they do not model the scan-authenticate-associate process,
// so they cannot be used in experiments involving handover.
//
// <i>The agent</i> is what instructs the management layer to perform
// scanning, authentication and association. The management layer itself
// just carries out these commands by performing the scanning, authentication
// and association procedures, and reports back the results to the agent.
//
// The agent layer is currenly only present in the ~Ieee80211Nic with
// mgmtType = ~Ieee80211MgmtSTA NIC module,
// as an ~Ieee80211AgentSTA module. The managament entities in other NIC
// variants do not have as much freedom as to need an agent to control them.
//
// By modifying or replacing the agent, one can alter the dynamic behaviour
// of STAs in the network, for example implement different handover strategies.
//
// <b>Limitations</b>
//
// See the documentation of ~Ieee80211Mac for features unsupported by this
// model.
//
//# further details about the implementation: what is modelled and what is
//# not (beacons, auth, ...), communication between modules, frame formats,
//# ...
//
// <b>Notes</b>
//
// If you are new to 802.11, reading an
// <a href="http://www.wi-fiplanet.com/tutorials/article.php/1216351" target="_blank">introductory material</a>
// before the IEEE standard can help.
//

package inet.linklayer.ieee80211;

