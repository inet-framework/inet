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
import inet.networklayer.common.IpProtocolId;
import inet.networklayer.contract.NetworkHeaderBase;

cplusplus {{
#include "inet/common/ProtocolGroup.h"
}}

namespace inet;

class WiseRouteHeader extends NetworkHeaderBase
{
    L3Address   srcAddr;
    L3Address   destAddr;
    unsigned long seqNum = 0;
    int     isFlood;
    int     nbHops;
    IpProtocolId     protocolId;
    L3Address finalDestAddr;
    L3Address initialSrcAddr;
    B         payloadLengthField;
}

cplusplus(WiseRouteHeader) {{
  public:
    virtual L3Address getSourceAddress() const override { return L3Address(getSrcAddr()); }
    virtual void setSourceAddress(const L3Address& address) override { setSrcAddr(address); }
    virtual L3Address getDestinationAddress() const override { return L3Address(getDestAddr()); }
    virtual void setDestinationAddress(const L3Address& address) override { setDestAddr(address); }
    virtual const Protocol *getProtocol() const override { return ProtocolGroup::ipprotocol.findProtocol(getProtocolId()); }
    virtual void setProtocol(const Protocol *protocol) override { setProtocolId(static_cast<IpProtocolId>(ProtocolGroup::ipprotocol.getProtocolNumber(protocol))); }
    virtual bool isFragment() const override { return false; }
}}

