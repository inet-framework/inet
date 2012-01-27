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

#include "INETDefs.h"

#include "DYMO_DataQueue.h"
#include "IPv4InterfaceData.h"
#include "Ieee802Ctrl_m.h"
#include "IPv4ControlInfo.h"
#include "IPv4.h"

std::ostream& operator<<(std::ostream& os, const DYMO_QueuedData& o)
{
    os << "[ ";
    os << "destAddr: " << o.destAddr;
    os << " ]";

    return os;
}

DYMO_DataQueue::DYMO_DataQueue(cSimpleModule *owner, int BUFFER_SIZE_PACKETS, int BUFFER_SIZE_BYTES) : dataQueueByteSize(0), BUFFER_SIZE_PACKETS(BUFFER_SIZE_PACKETS), BUFFER_SIZE_BYTES(BUFFER_SIZE_BYTES)
{
    moduleOwner = owner;
}

DYMO_DataQueue::~DYMO_DataQueue()
{
    while (!dataQueue.empty())
    {
        delete dataQueue.front().datagram;
        dataQueue.pop_front();
    }
}

const char* DYMO_DataQueue::getFullName() const
{
    return "DYMO_DataQueue";
}

std::string DYMO_DataQueue::info() const
{
    std::ostringstream ss;

    int total = dataQueue.size();
    ss << total << " queued datagrams: ";

    ss << "{" << std::endl;
    for (std::list<DYMO_QueuedData>::const_iterator iter = dataQueue.begin(); iter != dataQueue.end(); iter++)
    {
        DYMO_QueuedData e = *iter;
        ss << "  " << e << std::endl;
    }
    ss << "}";

    return ss.str();
}

std::string DYMO_DataQueue::detailedInfo() const
{
    return info();
}


void DYMO_DataQueue::queuePacket(const IPv4Datagram* datagram)
{
    IPv4Address destAddr = datagram->getDestAddress();

    ev << "Queueing data packet to " << destAddr << endl;
    dataQueue.push_back(DYMO_QueuedData(const_cast<IPv4Datagram *> (datagram), destAddr));
    dataQueueByteSize += datagram->getByteLength();

    // if buffer is full, force dequeueing of old packets
    while (((BUFFER_SIZE_PACKETS != -1) && ((int)dataQueue.size() > BUFFER_SIZE_PACKETS)) || ((BUFFER_SIZE_BYTES != -1) && (dataQueueByteSize > BUFFER_SIZE_BYTES)))
    {
        DYMO_QueuedData qd = dataQueue.front();
        dataQueue.pop_front();
        dataQueueByteSize -= qd.datagram->getByteLength();
        IPv4Address destAddr = qd.destAddr;
        delete qd.datagram;
        ev << "Forced dropping of data packet to " << destAddr << endl;
        //  ipLayer->reinjectDatagram(qd.datagram, IPv4::Hook::DROP);
    }
}

void DYMO_DataQueue::reinjectDatagramsTo(IPv4Address destAddr, int prefix, Result verdict, std::list<IPv4Datagram*> *datagrams)
{
    bool tryAgain = true;
    double delay = 0;
    while (tryAgain)
    {
#define ARP_DELAY 0.001
        tryAgain = false;
        for (std::list<DYMO_QueuedData>::iterator iter = dataQueue.begin(); iter != dataQueue.end(); iter++)
        {
            DYMO_QueuedData qd = *iter;
            if (qd.destAddr.prefixMatches(destAddr, prefix))
            {
                ev << "Dequeueing data packet to " << qd.destAddr << endl;
                dataQueueByteSize -= qd.datagram->getByteLength();
                dataQueue.erase(iter);
                if (verdict==ACCEPT)
                {
                    moduleOwner->send(qd.datagram, "to_ip");
                    delay += ARP_DELAY;
                }
                else if (verdict==DROP && datagrams != NULL)
                    datagrams->push_back( qd.datagram );
                else if (verdict==DROP)
                    delete qd.datagram;
                tryAgain = true;
                break;
            }
        }
    }
}

void DYMO_DataQueue::dequeuePacketsTo(IPv4Address destAddr, int prefix)
{
    reinjectDatagramsTo(destAddr, prefix, ACCEPT);
}

void DYMO_DataQueue::dropPacketsTo(IPv4Address destAddr, int prefix, std::list<IPv4Datagram*>* datagrams)
{
    reinjectDatagramsTo(destAddr, prefix, DROP, datagrams);
}



std::ostream& operator<<(std::ostream& os, const DYMO_DataQueue& o)
{
    os << o.info();
    return os;
}

