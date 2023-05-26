//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/ipv4/ipsec/IpSecProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/networklayer/ipv4/ipsec/IPsecAuthenticationHeader_m.h"
#include "inet/networklayer/ipv4/ipsec/IPsecEncapsulatingSecurityPayload_m.h"

namespace inet {
namespace ipsec {

Register_Protocol_Printer(&Protocol::ipsecAh, IpSecAhProtocolPrinter);
Register_Protocol_Printer(&Protocol::ipsecEsp, IpSecEspProtocolPrinter);

void IpSecAhProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const IPsecAuthenticationHeader>(chunk)) {
        context.typeColumn << "IpSec:AH";
        // TODO
        context.infoColumn << header;
    }
    else
        context.infoColumn << "(IpSec:AH) " << chunk;
}

void IpSecEspProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const IPsecEspHeader>(chunk)) {
        context.typeColumn << "IpSec:ESP";
        // TODO
        context.infoColumn << header;
    }
    else
        context.infoColumn << "(IpSec:ESP) " << chunk;
}

} // namespace ipsec
} // namespace inet

