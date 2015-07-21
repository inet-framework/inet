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

package inet.linklayer.ieee80211.thenewmac.mpdugeneration;

//
// This process generates one or more Mpdus from
// each outgoing Msdu or Mmpdu. If encryption is
// needed, the Mpdus are encrypted before being
// passed to be filtered for possible power save or
// CF queuing before tx.
//
simple Ieee80211MacPrepareMpdu
{
    parameters:
        string macsortsPackage = default("^.^.macsorts");
        string macmibPackage = default("^.^.macmib");

	gates:
	    inout fragMsdu; // From PM_Filter_STA: [FragConfirm] to PM_Filter_STA: [FragRequest]
	    inout mmpdu; // From Mlme_Sta_Services: [MmRequest] to Mlme_Sta_Services: [MmConfirm]
	    inout msdu; // From MSDU_from_LLC: [MsduRequest] to MSDU_from_LLC: [MsduConfirm]
}
