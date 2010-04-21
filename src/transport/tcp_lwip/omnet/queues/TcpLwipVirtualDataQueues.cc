//
// Copyright (C) 2004 Andras Varga
//               2010 Zoltan Bojthe
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


#include <omnetpp.h>

#include "TcpLwipVirtualDataQueues.h"

#include "TCPCommand_m.h"
#include "TcpLwipConnection.h"
#include "TCPSerializer.h"


Register_Class(TcpLwipVirtualDataSendQueue);

Register_Class(TcpLwipVirtualDataReceiveQueue);


/**
 * Ctor.
 */
TcpLwipVirtualDataSendQueue::TcpLwipVirtualDataSendQueue()
    :
    unsentTcpLayerBytesM(0)
{
}

/**
 * Virtual dtor.
 */
TcpLwipVirtualDataSendQueue::~TcpLwipVirtualDataSendQueue()
{
}

/**
 * set connection queue.
 */
void TcpLwipVirtualDataSendQueue::setConnection(TcpLwipConnection *connP)
{
    unsentTcpLayerBytesM = 0;
    TcpLwipSendQueue::setConnection(connP);
}

/**
 * Called on SEND app command, it inserts in the queue the data the user
 * wants to send. Implementations of this abstract class will decide
 * what this means: copying actual bytes, just increasing the
 * "last byte queued" variable, or storing cMessage object(s).
 * The msg object should not be referenced after this point (sendQueue may
 * delete it.)
 */
void TcpLwipVirtualDataSendQueue::enqueueAppData(cPacket *msgP)
{
    ASSERT(msgP);

    int bytes = msgP->getByteLength();
    delete msgP;

    unsentTcpLayerBytesM += bytes;
}

int TcpLwipVirtualDataSendQueue::getBytesForTcpLayer(void* bufferP, int bufferLengthP)
{
    ASSERT(bufferP);

    return (unsentTcpLayerBytesM > bufferLengthP) ? bufferLengthP : unsentTcpLayerBytesM;
}

/**
 * Remove msgLengthP bytes from the queue
 * But the TCP layer sometimes reread from this datapart (when data destroyed in IP Layer)
 *
 * called with return value of socket->send_data() if larger than 0
 */
void TcpLwipVirtualDataSendQueue::dequeueTcpLayerMsg(int msgLengthP)
{
    ASSERT(msgLengthP <= unsentTcpLayerBytesM);

    unsentTcpLayerBytesM -= msgLengthP;
}

/**
 * Utility function: returns how many bytes are available in the queue.
 */
ulong TcpLwipVirtualDataSendQueue::getBytesAvailable()
{
    return unsentTcpLayerBytesM; // TODO
}

/**
 * Called when the TCP wants to send or retransmit data, it constructs
 * a TCP segment which contains the data from the requested sequence
 * number range. The actually returned segment may contain less then
 * maxNumBytes bytes if the subclass wants to reproduce the original
 * segment boundaries when retransmitting.
 *
 * The method called from inside of send_callback()
 * The method called before called the send() to IP layer
 */
TCPSegment* TcpLwipVirtualDataSendQueue::createSegmentWithBytes(
        const void* tcpDataP, int tcpLengthP)
{
    ASSERT(tcpDataP);

    TCPSegment *tcpseg = new TCPSegment("tcp-segment");

    TCPSerializer().parse((const unsigned char *)tcpDataP, tcpLengthP, tcpseg);

    return tcpseg;
}

/**
 * Tells the queue that bytes up to (but NOT including) seqNum have been
 * transmitted and ACKed, so they can be removed from the queue.
 */
void TcpLwipVirtualDataSendQueue::discardUpTo(uint32 seqNumP)
{
    // nothing to do here
}

/**
 * Ctor.
 */
TcpLwipVirtualDataReceiveQueue::TcpLwipVirtualDataReceiveQueue()
    :
    bytesInQueueM(0)
{
}

/**
 * Virtual dtor.
 */
TcpLwipVirtualDataReceiveQueue::~TcpLwipVirtualDataReceiveQueue()
{
    // nothing to do here
}

/**
 * Add a connection queue.
 */
void TcpLwipVirtualDataReceiveQueue::setConnection(TcpLwipConnection *connP)
{
    ASSERT(connP);

    bytesInQueueM = 0;
    TcpLwipReceiveQueue::setConnection(connP);
}

/**
 * Called when a TCP segment arrives, it should extract the payload
 * from the segment and store it in the receive queue. The segment
 * object should be deleted any time.
 *
 * The method should return the number of bytes to copied to buffer.
 *
 * The method should fill the bufferP for data sending to NSC stack
 *
 * The method called before nsc_stack->if_receive_packet() called
 */
void TcpLwipVirtualDataReceiveQueue::insertBytesFromSegment(
        TCPSegment *tcpsegP, uint32 seqno, void* bufferP, size_t bufferLengthP)
{
    ASSERT(tcpsegP);
    ASSERT(bufferP);

//    return TCPSerializer().serialize(tcpsegP, (unsigned char *)bufferP, bufferLengthP);
}

/**
 * The method called when data received from NSC
 * The method should set status of the data in queue to received.
 *
 * The method called after socket->read_data() successfull
 */
void TcpLwipVirtualDataReceiveQueue::enqueueTcpLayerData(void* dataP, int dataLengthP)
{
    bytesInQueueM += dataLengthP;
}

/**
 * Should create a packet to be passed up to the app.
 * It should return NULL if there's no more data to be passed up --
 * this method is called several times until it returns NULL.
 *
 * the method called after socket->read_data() successful
 */
cPacket* TcpLwipVirtualDataReceiveQueue::extractBytesUpTo()
{
    ASSERT(connM);

    cPacket *dataMsg = NULL;
    if(bytesInQueueM)
    {
        IPvXAddress localAddr(ntohl(connM->pcbM->local_ip.addr));
        IPvXAddress remoteAddr(ntohl(connM->pcbM->remote_ip.addr));

        dataMsg = new cPacket("DATA");
        dataMsg->setKind(TCP_I_DATA);
        dataMsg->setByteLength(bytesInQueueM);
        TCPConnectInfo *tcpConnectInfo = new TCPConnectInfo();
        tcpConnectInfo->setConnId(connM->connIdM);
        tcpConnectInfo->setLocalAddr(localAddr);
        tcpConnectInfo->setRemoteAddr(remoteAddr);
        tcpConnectInfo->setLocalPort(connM->pcbM->local_port);
        tcpConnectInfo->setRemotePort(connM->pcbM->remote_port);
        dataMsg->setControlInfo(tcpConnectInfo);
        bytesInQueueM -= dataMsg->getByteLength();
    }
    return dataMsg;
}

/**
 * Returns the number of bytes currently buffered in queue.
 */
uint32 TcpLwipVirtualDataReceiveQueue::getAmountOfBufferedBytes()
{
    return bytesInQueueM;
}

/**
 * Returns the number of blocks currently buffered in queue.
 */
uint32 TcpLwipVirtualDataReceiveQueue::getQueueLength()
{
    return bytesInQueueM ? 1 : 0;
}

/**
 * Shows current queue status.
 */
void TcpLwipVirtualDataReceiveQueue::getQueueStatus()
{
    // TODO
}

/**
 * notify the queue about output messages
 *
 * called when connM send out a packet.
 * for read AckNo, if have
 */
void TcpLwipVirtualDataReceiveQueue::notifyAboutSending(const TCPSegment *tcpsegP)
{
    // nothing to do
}
