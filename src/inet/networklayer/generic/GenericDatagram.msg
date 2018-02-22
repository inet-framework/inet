//
// Copyright (C) 2012 Opensim Ltd.
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
import inet.common.TlvOptions;
import inet.networklayer.common.L3Address;
import inet.networklayer.contract.NetworkHeaderBase;

cplusplus {{
#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/Packet.h"
}}

import inet.networklayer.common.IpProtocolId;

namespace inet;

//
// Generic protocol TLV option types
//
enum GenericTlvOptionTypes
{
    GENERIC_TLVOPTION_NOP1 = 0;
    GENERIC_TLVOPTION_NOPN = 1;
    GENERIC_TLVOPTION_TLV_GPSR = 47;
};

//
// Represents a generic network datagram with generic network addresses. 
//
class GenericDatagramHeader extends NetworkHeaderBase
{
    L3Address srcAddr;
    L3Address destAddr;
    IpProtocolId protocolId = IP_PROT_NONE;
    short hopLimit;
    TlvOptions tlvOptions;
}

cplusplus(GenericDatagramHeader) {{
  public:
    virtual L3Address getSourceAddress() const override { return getSrcAddr(); }
    virtual void setSourceAddress(const L3Address& addr) override { setSrcAddr(addr); }
    virtual L3Address getDestinationAddress() const override { return getDestAddr(); }
    virtual void setDestinationAddress(const L3Address& addr) override { setDestAddr(addr); }
    virtual const Protocol *getProtocol() const override { return ProtocolGroup::ipprotocol.findProtocol(getProtocolId()); }
    virtual void setProtocol(const Protocol *protocol) override { setProtocolId((IpProtocolId)ProtocolGroup::ipprotocol.getProtocolNumber(protocol)); }
}}

