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
import inet.common.Units;
import inet.networklayer.common.IpProtocolId;
import inet.networklayer.contract.NetworkHeaderBase;

cplusplus {{
#include "inet/common/ProtocolGroup.h"
}}

namespace inet;

class ProbabilisticBroadcastHeader extends NetworkHeaderBase
{
    L3Address   srcAddr;
    L3Address   destAddr;
    long      id;    // Unique message ID ~ f(node address, unique number in node scope).
    int       nbHops;
    IpProtocolId   protocolId;
    simtime_t appTtl;   // Message time-to-live set by application.
    L3Address   initialSrcAddr;
    L3Address   finalDestAddr;
    B payloadLengthField;
}

cplusplus(ProbabilisticBroadcastHeader) {{
  public:
    virtual L3Address getSourceAddress() const override { return getSrcAddr(); }
    virtual void setSourceAddress(const L3Address& address) override { setSrcAddr(address); }
    virtual L3Address getDestinationAddress() const override { return getDestAddr(); }
    virtual void setDestinationAddress(const L3Address& address) override { setDestAddr(address); }
    virtual const Protocol *getProtocol() const override { return ProtocolGroup::ipprotocol.findProtocol(getProtocolId()); }
    virtual void setProtocol(const Protocol *protocol) override { setProtocolId(static_cast<IpProtocolId>(ProtocolGroup::ipprotocol.getProtocolNumber(protocol))); }
    virtual bool isFragment() const override { return false; }
}}

