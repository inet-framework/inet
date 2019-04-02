/*
 * Copyright (C) 2006 Christoph Sommer
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "inet/common/INETDefs.h"

#include "inet/routing/extras/dymo_fau/DYMO_DataQueue.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/networklayer/ipv4/Ipv4.h"
#include "inet/networklayer/common/L3Tools.h"

namespace inet {

namespace inetmanet {

std::ostream& operator<<(std::ostream& os, const DYMO_QueuedData& o)
{
    os << "[ ";
    os << "destAddr: " << o.destAddr;
    os << " ]";

    return os;
}

DYMO_DataQueue::DYMO_DataQueue(cSimpleModule *owner, int BUFFER_SIZE_PACKETS, int BUFFER_SIZE_BYTES) : dataQueueByteSize(0), BUFFER_SIZE_PACKETS(BUFFER_SIZE_PACKETS), BUFFER_SIZE_BYTES(BUFFER_SIZE_BYTES)
{
    moduleOwner = dynamic_cast<ManetRoutingBase *>(owner);
}

DYMO_DataQueue::~DYMO_DataQueue()
{
    if (moduleOwner->isInMacLayer()) {
        while (!dataQueue.empty()) {
            delete dataQueue.front().datagram;
            dataQueue.pop_front();
        }
    }
}

const char* DYMO_DataQueue::getFullName() const
{
    return "DYMO_DataQueue";
}

std::string DYMO_DataQueue::str() const
{
    std::ostringstream ss;

    int total = dataQueue.size();
    ss << total << " queued datagrams: ";

    ss << "{" << std::endl;
    for (auto e : dataQueue)
    {
        
        ss << "  " << e << std::endl;
    }
    ss << "}";

    return ss.str();
}


void DYMO_DataQueue::queuePacket(Packet* datagram)
{
    const auto& networkHeader = getNetworkProtocolHeader(datagram);
    const L3Address& destAddr = networkHeader->getDestinationAddress();
    //const L3Address& sourceAddr = networkHeader->getSourceAddress();

    //IPv4Address destAddr = datagram->getDestAddress();

    EV_INFO << "Queueing data packet to " << destAddr << endl;
    dataQueue.push_back(DYMO_QueuedData(datagram, destAddr));
    dataQueueByteSize += datagram->getByteLength();

    // if buffer is full, force dequeueing of old packets
    while (((BUFFER_SIZE_PACKETS != -1) && ((int)dataQueue.size() > BUFFER_SIZE_PACKETS)) || ((BUFFER_SIZE_BYTES != -1) && (dataQueueByteSize > BUFFER_SIZE_BYTES)))
    {
        DYMO_QueuedData qd = dataQueue.front();
        dataQueue.pop_front();
        dataQueueByteSize -= qd.datagram->getByteLength();
        L3Address destAddr = qd.destAddr;
        delete qd.datagram;
        EV_INFO << "Forced dropping of data packet to " << destAddr << endl;
        //  ipLayer->reinjectDatagram(qd.datagram, IPv4::Hook::DROP);
    }
}

void DYMO_DataQueue::reinjectDatagramsTo(L3Address destAddr, int prefix, Result verdict, std::list<Packet*> *datagrams)
{
    bool tryAgain = true;
    double delay = 0;
    while (tryAgain)
    {
#define ARP_DELAY 0.001
        tryAgain = false;
        for (auto iter = dataQueue.begin(); iter != dataQueue.end(); iter++)
        {
            DYMO_QueuedData qd = *iter;
            if (qd.destAddr.toIpv4().prefixMatches(destAddr.toIpv4(), prefix))
            {
                EV_INFO << "Dequeueing data packet to " << qd.destAddr << endl;
                dataQueueByteSize -= qd.datagram->getByteLength();
                dataQueue.erase(iter);
                if (verdict==ACCEPT)
                {
                    if (moduleOwner->getNetworkProtocol())
                        moduleOwner->getNetworkProtocol()->reinjectQueuedDatagram(qd.datagram);
                    else
                        moduleOwner->sendDelayed(qd.datagram, delay, "ipOut");
                    delay += ARP_DELAY;
                }
                else if (verdict==DROP && datagrams != nullptr)
                    datagrams->push_back( qd.datagram );
                else if (verdict==DROP) {
                    if (moduleOwner->getNetworkProtocol())
                        moduleOwner->getNetworkProtocol()->dropQueuedDatagram(qd.datagram);
                    else
                        delete qd.datagram;

                }
                tryAgain = true;
                break;
            }
        }
    }
}

void DYMO_DataQueue::dequeuePacketsTo(L3Address destAddr, int prefix)
{
    reinjectDatagramsTo(destAddr, prefix, ACCEPT);
}

void DYMO_DataQueue::dropPacketsTo(L3Address destAddr, int prefix, std::list<Packet*>* datagrams)
{
    reinjectDatagramsTo(destAddr, prefix, DROP, datagrams);
}



std::ostream& operator<<(std::ostream& os, const DYMO_DataQueue& o)
{
    os << o.str();
    return os;
}

} // namespace inetmanet

} // namespace inet

