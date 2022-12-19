//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Units;
import inet.networklayer.contract.NetworkHeaderBase;
import inet.networklayer.common.L3Address;
import inet.networklayer.common.IpProtocolId;

cplusplus {{
#include "inet/common/ProtocolGroup.h"
}}

namespace inet;

class FloodingHeader extends NetworkHeaderBase
{
    L3Address     srcAddr;
    L3Address     destAddr;
    int           ttl = 1;    // time to live field
    unsigned long seqNum = 0; // sequence number
    IpProtocolId  protocolId;
    B             payloadLengthField;
}

cplusplus(FloodingHeader) {{
  public:
    virtual L3Address getSourceAddress() const override { return L3Address(getSrcAddr()); }
    virtual void setSourceAddress(const L3Address& address) override { setSrcAddr(address); }
    virtual L3Address getDestinationAddress() const override { return L3Address(getDestAddr()); }
    virtual void setDestinationAddress(const L3Address& address) override { setDestAddr(address); }
    virtual const Protocol *getProtocol() const override { return ProtocolGroup::getIpProtocolGroup()->findProtocol(getProtocolId()); }
    virtual void setProtocol(const Protocol *protocol) override { setProtocolId(static_cast<IpProtocolId>(ProtocolGroup::getIpProtocolGroup()->getProtocolNumber(protocol))); }
    virtual bool isFragment() const override { return false; }
}}
