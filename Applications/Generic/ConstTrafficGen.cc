//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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

/*
    file: ConstTrafficGen.cc
        Purpose: Traffic Generator that sends out
                 constant packets in constant time intervals
    author: Jochen Reber
*/


#include <omnetpp.h>
#include "ConstTrafficGen.h"
#include "IPControlInfo_m.h"
#include "IPDatagram.h"
#include "ICMP.h"

const int GEN_NO = 2;

Define_Module_Like ( ConstTrafficGen, GeneratorAppOut );

void ConstTrafficGen::initialize()
{
    generationTime = par("generationTime");
    packetSize = par("generationSize");
    usesTCPProt = par("tcpProtocol");
}

// FIXME this is nearly the same as BurstGenerator -- KILL one of them!!!!!
void ConstTrafficGen::activity()
{
    int contCtr = id()*10000+100;

    while(true)
    {
        wait(generationTime);

        cPacket *transportPacket = new cPacket("burstgen-pk");
        transportPacket->setLength(packetSize);
        transportPacket->addPar("content") = contCtr++;

        IPAddress destAddr = chooseDestAddr();

        IPControlInfo *controlInfo = new IPControlInfo();
        controlInfo->setDestAddr(destAddr);
        controlInfo->setProtocol(usesTCPProt ? IP_PROT_TCP : IP_PROT_UDP);
        transportPacket->setControlInfo(controlInfo);

        ev << "Constant Traffic Generator: sending packet:\n"
           << " Content: " << int(transportPacket->par("content"))
           << " Bitlength: " << int(transportPacket->length())
           << "   Time: " << simTime()
           << "\nDest: " << destAddr
           << "\n";

        send(transportPacket, "out");

    } // end while
}

/* random destination addresses
    based on test2.irt */
IPAddress ConstTrafficGen::chooseDestAddr()
{
    switch(intrand(2))
    {
        // computer no 1
        case 0: return IPAddress("10.10.0.1");
        // computer no 2
        case 1: return IPAddress("10.10.0.2");
        // DL sink
        case 2: return IPAddress("10.20.0.3");
        // local loopback
        case 3: return IPAddress("127.0.0.1");
        // IP address doesn't exist
        case 4: return IPAddress("10.30.0.5");
    }
}

