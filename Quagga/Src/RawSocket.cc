
#include <omnetpp.h>

#include "RawSocket.h"

#include "SocketMsg.h"

#include "IPControlInfo.h"
#include "IPDatagram.h"
#include "oppsim_kernel.h"

#include "RoutingTableAccess.h"

#include "glue.h"

RawSocket::RawSocket(int userId, int protocol)
{
    EV << "creating new RawSocket for protocol=" << protocol << ", user supplied id=" << userId << endl;

    this->userId = userId;
    this->protocol = protocol;
}

int RawSocket::send(const struct msghdr *message, int flags)
{
    ASSERT(message->msg_iovlen == 2);
    ASSERT(message->msg_iov[0].iov_len == sizeof (struct ip));

    // msg_iov[0] contains IP header
    // msg_iov[1] contains data

    bool dontroute = ((flags & MSG_DONTROUTE));
    bool hdrincl = getHdrincl();

    ASSERT(hdrincl);
    ASSERT(flags == 0 || flags == MSG_DONTROUTE);

    // get destination address

    ASSERT(message->msg_namelen == sizeof(sockaddr_in));
    struct sockaddr_in *inaddr = (sockaddr_in*)message->msg_name;
    IPAddress destAddr = IPAddress(ntohl(inaddr->sin_addr.s_addr));
    int port = ntohs(inaddr->sin_port);

    EV << "destAddr=" << destAddr << " port=" << port << endl;

    ASSERT(port == 0);

    // message (data) length

    int length = message->msg_iov[1].iov_len;

    // create payload

    SocketMsg *msg = new SocketMsg("data");

    msg->setDataFromBuffer(message->msg_iov[1].iov_base, length);
    msg->setByteLength(length);

    // create controlInfo

    IPControlInfo *ipControlInfo = new IPControlInfo();

    ipControlInfo->setDestAddr(destAddr);
    ipControlInfo->setProtocol(protocol);

    if(hdrincl)
    {
        // set extended attributes in ipControlInfo

        struct ip *iph = (struct ip *)message->msg_iov[0].iov_base;

        ASSERT(iph->ip_hl == (sizeof(struct ip) / sizeof(u_int32_t))); // options wouldn't be reconstructed on reception

        ASSERT(IPAddress(ntohl(iph->ip_dst.s_addr)) == destAddr);   // already set, values should match
        ASSERT(inet_protocol(iph->ip_p) == protocol);               // dtto

        ipControlInfo->setSrcAddr(IPAddress(ntohl(iph->ip_src.s_addr)));
        ipControlInfo->setDiffServCodePoint(iph->ip_tos);
        ipControlInfo->setTimeToLive(iph->ip_ttl);
    }

    if(destAddr.isMulticast())
    {
        ipControlInfo->setInterfaceId(multicastOutputInterfaceId);
    }
    else
    {
        // grant DONTROUTE option (?)
    }

    msg->setControlInfo(ipControlInfo);

    sendToIP(msg);

    return length;

}

void RawSocket::sendToIP(cMessage *msg)
{
    if (!gateToIP)
        opp_error("RawSocket: setOutputGate() must be invoked before socket can be used");

    check_and_cast<cSimpleModule *>(gateToIP->ownerModule())->send(msg, gateToIP);
}
