//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.networklayer.common.IpProtocolId;
import inet.networklayer.contract.NetworkHeaderBase;

cplusplus {{
#include "inet/common/ProtocolGroup.h"
}}

namespace inet;

enum WiseRouteMsgType {
    DATA = 1;
    ROUTE_FLOOD = 2;
};

class WiseRouteHeader extends NetworkHeaderBase
{
    L3Address   srcAddr;
    L3Address   destAddr;
    unsigned long seqNum = 0;
    int     isFlood;
    int     nbHops;
    WiseRouteMsgType     headerKind = static_cast<WiseRouteMsgType>(-1);
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
    virtual const Protocol *getProtocol() const override { return ProtocolGroup::getIpProtocolGroup()->findProtocol(getProtocolId()); }
    virtual void setProtocol(const Protocol *protocol) override { setProtocolId(static_cast<IpProtocolId>(ProtocolGroup::getIpProtocolGroup()->getProtocolNumber(protocol))); }
    virtual bool isFragment() const override { return false; }
}}
