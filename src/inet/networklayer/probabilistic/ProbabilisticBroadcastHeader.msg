//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
    virtual const Protocol *getProtocol() const override { return ProtocolGroup::getIpProtocolGroup()->findProtocol(getProtocolId()); }
    virtual void setProtocol(const Protocol *protocol) override { setProtocolId(static_cast<IpProtocolId>(ProtocolGroup::getIpProtocolGroup()->getProtocolNumber(protocol))); }
    virtual bool isFragment() const override { return false; }
}}
