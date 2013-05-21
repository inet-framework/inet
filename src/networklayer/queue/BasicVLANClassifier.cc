//
// Copyright (C) 2012 Kyeong Soo (Joseph) Kim
// Copyright (C) 2005 Andras Varga
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


#include "BasicVLANClassifier.h"
#include "EtherFrame_m.h"
//#include "IPDatagram.h"
//#ifndef WITHOUT_IPv6
//#include "IPv6Datagram.h"
//#endif

Register_Class(BasicVLANClassifier);

void BasicVLANClassifier::initializeIndexTable(const char *str)
{
    cStringTokenizer tokenizer(str);
    while (tokenizer.hasMoreTokens())
    {
        if (numQueues >= maxNumQueues)
        {
            throw cRuntimeError("%s::initializeIndexTable: Exceeds the maximum number of queues", getFullPath().c_str());
        }
        else
        {
            const char *token = tokenizer.nextToken();
            indexTable[atof(vid)] = numQueues++;
        }
    }
}

int BasicVLANClassifier::getNumQueues()
{
//    return indexTable.size();
    return numQueues;
}

int BasicVLANClassifier::classifyPacket(cMessage *msg)
{
    if (dynamic_cast<EthernetIIFrameWithVLAN *>(msg))
    {
        // Ethernet Frame with IEEE 802.1Q VLAN tag: map VID to queue number
        EthernetIIFrameWithVLAN *vlanFrame = (EthernetIIFrameWithVLAN *)msg;
        int vid = vlanFrame->getVid();
        return classifyByVID(vid);
    }
    else
    {
        throw cRuntimeError("%s::classifyPacket: Received a message which is not Ethernet Frame with VLAN tag", getFullPath().c_str());
        // We cannot use error() because this class is not derived from cModule.
    }
}

int BasicVLANClassifier::classifyByVID(int vid)
{
    QueueIndexTable::iterator it = indexTable.find(vid);
    if (it != indexTable.end())
    {
        return it->second;
    }
    else
    {
        if (numQueues >= maxNumQueues)
        {
            throw cRuntimeError("%s::classifyByVID: Exceeds the maximum number of queues", getFullPath().c_str());
        }
        else
        {
            return (indexTable[vid] = numQueues++);
        }
    }
}

