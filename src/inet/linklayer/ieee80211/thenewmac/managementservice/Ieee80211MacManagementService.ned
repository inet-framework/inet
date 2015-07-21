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

package inet.linklayer.ieee80211.thenewmac.managementservice;

//
// This process handles requests sequentially.
// Start, join, powermgt, scan, re/dis/associate
// and deauthenticate must be sequential.
// It is possible to have multiple authentication
// sequences in progress concurrently. To allow
// this, AuthReq_Service in the MLME block
// would have to cache challenge text and
// match responses to cached request info.
//
module Ieee80211MacManagementService
{
    gates:
        inout mmgtToMgt;
        input mmgtFromMgt;
        inout getSet;
        
    submodules:
        mib: Ieee80211MacMib;
        mlmeRequests: Ieee80211MacMlmeRequests {
            @display("p=50,160");
        }
        mlmeIndications: Ieee80211MacMlmeIndications {
            @display("p=167,160");
        }

    connections:
        // Inner links
        mib.mres --> mlmeRequests.mres;
        mlmeRequests.toMgt <--> mmgtToMgt;
        mlmeIndications.fromMgt <-- mmgtFromMgt;
        
        mib.getSet <--> getSet;
}
