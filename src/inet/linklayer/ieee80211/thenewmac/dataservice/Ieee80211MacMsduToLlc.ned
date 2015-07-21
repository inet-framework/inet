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

package inet.linklayer.ieee80211.thenewmac.dataservice;

//
// This process runs when reception is successfully
// completed on an MSDU addressed to the local
// LLC entity. This process extracts the appropriate
// address and status info, removes the MAC header
// from the MSDU data field (the FCS and IV/ICV are
// removed much earlier in reception handling), and
// generates the indication to LLC. Reception status
// is always "successful" because a receive error
// causes the MSDU to be discarded before reaching
// MAC Data Service.
//
simple Ieee80211MacMsduToLlc
{
    gates:
        input rxMsdu;
        output toLlc;
}
