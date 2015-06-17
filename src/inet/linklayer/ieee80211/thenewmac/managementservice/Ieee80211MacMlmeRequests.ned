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

package inet.linklayer.ieee80211.thenewmac.managementservice;

//
// This process tracks the synchronization state
// of the station as Idle (not part of any Bss),
// Ibss (started or joined an independent Bss), Bss
// (joined an infrastructure Bss), or Ap (started an
// infrastructure Bss). Mlme operation requests
// invalid in the current state are rejected here,
// while valid requests are passed to the Mlme block
// for processing. This simplifies process flow
// and signal saving in the Mlme block, because only
//meaningful Mlme requests arrive for handling.
//
simple Ieee80211MacMlmeRequests
{
    gates:
        input mres;
        inout toMgt;
}
