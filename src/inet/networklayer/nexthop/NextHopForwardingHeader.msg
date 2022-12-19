//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
// Next hop forwarding TLV option types
//
enum NextHopTlvOptionTypes
{
    NEXTHOP_TLVOPTION_NOP1 = 0;
    NEXTHOP_TLVOPTION_NOPN = 1;
    NEXTHOP_TLVOPTION_TLV_GPSR = 47;
};

//
// Next hop forwarding datagram header using generic network addresses.
//
class NextHopForwardingHeader extends NetworkHeaderBase
{
    L3Address srcAddr;
    L3Address destAddr;
    IpProtocolId protocolId = IP_PROT_NONE;
    short hopLimit;
    B payloadLengthField;
    TlvOptions tlvOptions;
}

cplusplus(NextHopForwardingHeader) {{
  public:
    virtual L3Address getSourceAddress() const override { return getSrcAddr(); }
    virtual void setSourceAddress(const L3Address& addr) override { setSrcAddr(addr); }
    virtual L3Address getDestinationAddress() const override { return getDestAddr(); }
    virtual void setDestinationAddress(const L3Address& addr) override { setDestAddr(addr); }
    virtual const Protocol *getProtocol() const override { return ProtocolGroup::getIpProtocolGroup()->findProtocol(getProtocolId()); }
    virtual void setProtocol(const Protocol *protocol) override { setProtocolId(static_cast<IpProtocolId>(ProtocolGroup::getIpProtocolGroup()->getProtocolNumber(protocol))); }
    virtual bool isFragment() const override { return false; }
}}
