//
// Copyright (C) 2012 Andras Varga
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

import inet.common.INETDefs;
import inet.networklayer.contract.NetworkHeaderBase;

namespace inet;

class noncobject L3Address;

class ProbabilisticBroadcastHeader extends NetworkHeaderBase
{
    @customize(true);

    L3Address   srcAddr;
    L3Address   destAddr;
    long      id;    // Unique message ID ~ f(node address, unique number in node scope).
    int       nbHops;
    int       protocolId @override = -1;
    simtime_t appTtl;   // Message time-to-live set by application.
    L3Address   initialSrcAddr;
    L3Address   finalDestAddr;
}
