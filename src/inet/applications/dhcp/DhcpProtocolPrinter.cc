//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/dhcp/DhcpProtocolPrinter.h"

#include "inet/applications/dhcp/DhcpMessage_m.h"
#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"

namespace inet {

Register_Protocol_Printer(&Protocol::dhcp, DhcpProtocolPrinter);

static const char *messageTypeName(DhcpMessageType type)
{
    static cEnum *e = cEnum::find("inet::DhcpMessageType");
    const char *name = (e != nullptr) ? e->getStringFor(type) : nullptr;
    return name != nullptr ? name : "DHCP";
}

void DhcpProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    const auto& dhcp = dynamicPtrCast<const DhcpMessage>(chunk);
    if (dhcp == nullptr) {
        context.infoColumn << "(DHCP) " << chunk;
        return;
    }

    const DhcpOptions& opt = dhcp->getOptions();
    bool hasType = opt.getMessageType() != static_cast<DhcpMessageType>(-1);

    // Headline: human-readable summary line.
    std::ostream& out = context.infoColumn;
    out << (hasType ? messageTypeName(opt.getMessageType())
                    : (dhcp->getOp() == BOOTREQUEST ? "BOOTREQUEST" : "BOOTREPLY"));
    out << " xid=0x" << std::hex << dhcp->getXid() << std::dec;
    if (!dhcp->getCiaddr().isUnspecified())
        out << " ciaddr=" << dhcp->getCiaddr();
    if (!dhcp->getYiaddr().isUnspecified())
        out << " yiaddr=" << dhcp->getYiaddr();
    if (!dhcp->getGiaddr().isUnspecified())
        out << " giaddr=" << dhcp->getGiaddr();
    if (!dhcp->getChaddr().isUnspecified())
        out << " chaddr=" << dhcp->getChaddr();

    // Per-option TLV breakdown: only the options actually present, each tagged
    // with its DHCP option code. The presence tests mirror DhcpMessageSerializer.
    if (hasType)
        out << " opt[" << DHCP_MSG_TYPE << "]msgType=" << messageTypeName(opt.getMessageType());
    if (opt.getHostName() != nullptr && opt.getHostName()[0] != '\0')
        out << " opt[" << HOSTNAME << "]hostName=" << opt.getHostName();
    if (opt.getParameterRequestListArraySize() > 0) {
        out << " opt[" << PARAM_LIST << "]paramList=";
        for (size_t i = 0; i < opt.getParameterRequestListArraySize(); ++i)
            out << (i == 0 ? "" : ",") << static_cast<int>(opt.getParameterRequestList(i));
    }
    if (!opt.getClientIdentifier().isUnspecified())
        out << " opt[" << CLIENT_ID << "]clientId=" << opt.getClientIdentifier();
    if (!opt.getRequestedIp().isUnspecified())
        out << " opt[" << REQUESTED_IP << "]requestedIp=" << opt.getRequestedIp();
    if (!opt.getSubnetMask().isUnspecified())
        out << " opt[" << SUBNET_MASK << "]subnetMask=" << opt.getSubnetMask();
    if (opt.getRouterArraySize() > 0) {
        out << " opt[" << ROUTER << "]router=";
        for (size_t i = 0; i < opt.getRouterArraySize(); ++i)
            out << (i == 0 ? "" : ",") << opt.getRouter(i);
    }
    if (opt.getDnsArraySize() > 0) {
        out << " opt[" << DNS << "]dns=";
        for (size_t i = 0; i < opt.getDnsArraySize(); ++i)
            out << (i == 0 ? "" : ",") << opt.getDns(i);
    }
    if (opt.getNtpArraySize() > 0) {
        out << " opt[" << NTP_SRV << "]ntp=";
        for (size_t i = 0; i < opt.getNtpArraySize(); ++i)
            out << (i == 0 ? "" : ",") << opt.getNtp(i);
    }
    if (!opt.getServerIdentifier().isUnspecified())
        out << " opt[" << SERVER_ID << "]serverId=" << opt.getServerIdentifier();
    if (!opt.getRenewalTime().isZero())
        out << " opt[" << RENEWAL_TIME << "]T1=" << opt.getRenewalTime() << "s";
    if (!opt.getRebindingTime().isZero())
        out << " opt[" << REBIND_TIME << "]T2=" << opt.getRebindingTime() << "s";
    if (!opt.getLeaseTime().isZero())
        out << " opt[" << LEASE_TIME << "]leaseTime=" << opt.getLeaseTime() << "s";
}

} // namespace inet
