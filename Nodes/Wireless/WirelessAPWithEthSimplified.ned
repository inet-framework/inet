//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


import
    "MACRelayUnit",
    "EtherMAC",
    "NotificationBoard",
    "EthernetInterface",
    "Ieee80211NicAPSimplified";


//
// Models a 802.11 Access Point with Ethernet ports, but using a simplified NIC
// that does not support handovers. It can be used in conjunction with
// WirelessHostSimplified, or any other host model containing WirelessNicSTASimplified.
//
// @see WirelessAP, WirelessAPSimplified, WirelessAPWithEth, WirelessAPWithEthSimplified
// @see WirelessHost, WirelessHostSimplified
// @see MobileHost, MFMobileHost
//
module WirelessAPWithEthSimplified
    parameters:
        relayUnitType: string; // type of the MACRelayUnit (MACRelayUnitNP, MACRelayUnitPP, etc)
    gates:
        in: radioIn;
        in: ethIn[];
        out: ethOut[];
    submodules:
        relayUnit: relayUnitType like MACRelayUnit;
            gatesizes:
                lowerLayerIn[sizeof(ethIn)+1],
                lowerLayerOut[sizeof(ethIn)+1];
            display: "i=greenbox;p=169,124";
        wlan: Ieee80211NicAPSimplified;
            display: "p=89,225;q=queue;i=block/ifcard,#ff8000";
        eth: EtherMAC[sizeof(ethIn)];
            parameters:
                promiscuous = true,
                txQueueLimit = 1000, // increase if needed
                queueModule = "";
            display: "i=block/ifcard;p=174,225,row;q=queue";
        notificationBoard: NotificationBoard;
            display: "p=78,60;i=block/control";
        mobility: NullMobility;
            display: "p=71,121;i=block/cogwheel_s";
    connections nocheck:
        radioIn --> wlan.radioIn display "m=s";
        wlan.uppergateIn <-- relayUnit.lowerLayerOut[0];
        wlan.uppergateOut --> relayUnit.lowerLayerIn[0];
        for i=0..sizeof(ethIn)-1 do
            eth[i].upperLayerIn <-- relayUnit.lowerLayerOut[i+1];
            eth[i].upperLayerOut --> relayUnit.lowerLayerIn[i+1];
            eth[i].physIn <-- ethIn[i];
            eth[i].physOut --> ethOut[i];
        endfor;
endmodule

