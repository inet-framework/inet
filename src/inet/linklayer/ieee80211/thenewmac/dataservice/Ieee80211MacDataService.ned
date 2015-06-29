//
// Copyright (C) 2015 OpenSim Ltd.
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

package inet.linklayer.ieee80211.thenewmac.dataservice;

// Includes request validation and add/remove MAC headers.
//
module Ieee80211MacDataService
{
    gates:
        inout tsdu;
        input rsdu;
        inout macSapMsduFromLlc;
        output macSapMsduToLlc;

    submodules:
        msduToLlc: Ieee80211MacMsduToLlc;
        msduFromLlc: Ieee80211MacMsduFromLlc;

    connections:
        msduFromLlc.txMsdu <--> tsdu;
        msduFromLlc.fromLlc <--> macSapMsduFromLlc;
        msduToLlc.toLlc --> macSapMsduToLlc;
        msduToLlc.rxMsdu <-- rsdu;

}
