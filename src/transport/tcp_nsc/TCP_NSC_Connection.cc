//
// Copyright (C) 2006 Sam Jansen, Andras Varga
// Copyright (C) 2009 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include "TCP_NSC_Connection.h"

#include "headers/defs.h"   // for endian macros
#include <sim_interface.h> // NSC header
#include "headers/tcp.h"
#include "TCP_NSC.h"
#include "TCP_NSC_Queues.h"
#include "TCPCommand_m.h"
#include "TCPIPchecksum.h"
#include "TCPSegment.h"
#include "TCPSerializer.h"

#include <assert.h>
#include <dlfcn.h>
#include <netinet/in.h>


// macro for normal ev<< logging (note: deliberately no parens in macro def)
#define tcpEV ((ev.isDisabled())||(TCP_NSC::testingS)) ? ev : ev


struct nsc_iphdr
{
#if BYTE_ORDER == LITTLE_ENDIAN
    unsigned int ihl:4;
    unsigned int version:4;
#elif BYTE_ORDER == BIG_ENDIAN
    unsigned int version:4;
    unsigned int ihl:4;
#else
# error "Please check BYTE_ORDER declaration"
#endif
    uint8_t tos;
    uint16_t tot_len;
    uint16_t id;
    uint16_t frag_off;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t check;
    uint32_t saddr;
    uint32_t daddr;
    /*The options start here. */
} __attribute__((packed));

TCP_NSC_Connection::TCP_NSC_Connection()
    :
    connIdM(-1),
    appGateIndexM(-1),
    pNscSocketM(NULL),
    sentEstablishedM(false),
    onCloseM(false),
    disconnectCalledM(false),
    isListenerM(false),
    tcpWinSizeM(65536),
    tcpNscM(NULL),
    receiveQueueM(NULL),
    sendQueueM(NULL)
{
}

// create a TCP_I_ESTABLISHED msg
cMessage* TCP_NSC_Connection::createEstablishedMsg()
{
    if (sentEstablishedM)
        return NULL;

    cMessage *msg = new cMessage("TCP_I_ESTABLISHED");
    msg->setKind(TCP_I_ESTABLISHED);

    TCPConnectInfo *tcpConnectInfo = new TCPConnectInfo();

    tcpConnectInfo->setConnId(connIdM);
    tcpConnectInfo->setLocalAddr(inetSockPairM.localM.ipAddrM);
    tcpConnectInfo->setRemoteAddr(inetSockPairM.remoteM.ipAddrM);
    tcpConnectInfo->setLocalPort(inetSockPairM.localM.portM);
    tcpConnectInfo->setRemotePort(inetSockPairM.remoteM.portM);

    msg->setControlInfo(tcpConnectInfo);
    //tcpMain->send(estmsg, "appOut", appGateIndex);
    return msg;
}

void TCP_NSC_Connection::connect(INetStack &stackP, SockPair &inetSockPairP, SockPair &nscSockPairP)
{
    ASSERT(!pNscSocketM);
    pNscSocketM = stackP.new_tcp_socket();
    ASSERT(pNscSocketM);

    // TODO NSC not yet implements bind (for setting localport)

    ASSERT(sendQueueM);
    ASSERT(receiveQueueM);

    sendQueueM->setConnection(this);
    receiveQueueM->setConnection(this);

    onCloseM = false;

    pNscSocketM->connect(nscSockPairM.remoteM.ipAddrM.str().c_str(), nscSockPairM.remoteM.portM);

    struct sockaddr_in sockAddr;
    size_t sockAddrLen = sizeof(sockAddr);
    pNscSocketM->getsockname((sockaddr*)&sockAddr, &sockAddrLen);
    nscSockPairP.localM.ipAddrM.set(IPv4Address(sockAddr.sin_addr.s_addr));
    nscSockPairP.localM.portM = ntohs(sockAddr.sin_port);
/*
    // TODO: getpeername generate an assert!!!
    pNscSocketM->getpeername((sockaddr*)&sockAddr, &sockAddrLen);
    nscSockPairP.remoteM.ipAddrM.set(sockAddr.sin_addr.s_addr);
    nscSockPairP.remoteM.portM = ntohs(sockAddr.sin_port);
*/
}

void TCP_NSC_Connection::listen(INetStack &stackP, SockPair &inetSockPairP, SockPair &nscSockPairP)
{
    ASSERT(nscSockPairP.localM.portM != -1);
    ASSERT(!pNscSocketM);
    ASSERT(sendQueueM);
    ASSERT(receiveQueueM);

    isListenerM = true;
    pNscSocketM = stackP.new_tcp_socket();
    ASSERT(pNscSocketM);

    // TODO NSC not yet implements bind (for setting remote addr)

    sendQueueM->setConnection(this);
    receiveQueueM->setConnection(this);

    onCloseM = false;

    pNscSocketM->listen(nscSockPairP.localM.portM);

    struct sockaddr_in sockAddr;
    size_t sockAddrLen = sizeof(sockAddr);
    pNscSocketM->getsockname((sockaddr*)&sockAddr, &sockAddrLen);

    nscSockPairP.localM.ipAddrM.set(IPv4Address(sockAddr.sin_addr.s_addr));
    nscSockPairP.localM.portM = ntohs(sockAddr.sin_port);
    nscSockPairP.remoteM.ipAddrM = IPvXAddress();
    nscSockPairP.remoteM.portM = -1;
}

void TCP_NSC_Connection::send(cPacket *msgP)
{
    ASSERT(sendQueueM);
    sendQueueM->enqueueAppData(msgP);
}

void TCP_NSC_Connection::do_SEND()
{
    if (pNscSocketM)
    {
        ASSERT(sendQueueM);

        char buffer[4096];
        int allsent = 0;

        while (1)
        {
            int bytes = sendQueueM->getBytesForTcpLayer(buffer, sizeof(buffer));

            if (0 == bytes)
                break;

            int sent = pNscSocketM->send_data(buffer, bytes);

            if (sent > 0)
            {
                sendQueueM->dequeueTcpLayerMsg(sent);
                allsent += sent;
            }
            else
            {
                tcpEV << "TCP_NSC connection: " << connIdM << ": Error do sending, err is " << sent << "\n";
                break;

            }
        }
//        tcpEV << "do_SEND(): " << connIdM << " sent:" << allsent << ", unsent:" << sendQueueM->getBytesAvailable() << "\n";
        if (onCloseM && sendQueueM->getBytesAvailable()==0 && !disconnectCalledM)
        {
            disconnectCalledM = true;
            pNscSocketM->disconnect();
            cMessage *msg = new cMessage("CLOSED");
            msg->setKind(TCP_I_CLOSED);
            TCPCommand *ind = new TCPCommand();
            ind->setConnId(connIdM);
            msg->setControlInfo(ind);
            tcpNscM->send(msg, "appOut", appGateIndexM);
            //FIXME this connection never will be deleted, stayed in tcpNscM. Should delete later!
        }
    }
}

void TCP_NSC_Connection::close()
{
    onCloseM = true;
}

void TCP_NSC_Connection::abort()
{
    sendQueueM->dequeueTcpLayerMsg(sendQueueM->getBytesAvailable());
    close();
}

