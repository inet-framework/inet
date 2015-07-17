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

simple Ieee80211MacPmFilterSta
{
    parameters:
        string macsortsPackage = default("^.^.macsorts");
        string macmibPackage = default("^.^.macmib");

	gates:
	    inout fragMsdu; // To PrepareMpdu: [FragConfirm] from PrepareMpdu: [FraqRquest] 
	    inout mpdu; // From Tx_Coordination_sta: [AtimW, PduConfirm, CfPolled] to RxCoordination: [PduRequest]
	    inout pwrMgt;
}
