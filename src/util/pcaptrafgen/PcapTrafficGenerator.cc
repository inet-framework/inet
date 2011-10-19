//
// Copyright (C) 2011 OpenSim Ltd
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
// @author Zoltan Bojthe
//

#include <vector>

#include "PcapTrafficGenerator.h"

#include "InterfaceTableAccess.h"
#include "IPv4ControlInfo.h"
#include "IPv4Datagram.h"
#include "IPv4Route.h"
#include "RoutingTableAccess.h"

Define_Module(PcapTrafficGenerator);

void PcapTrafficGenerator::initialize()
{
    const char* filename = this->par("pcapFile").stringValue();
    const char* parserName = this->par("pcapParser").stringValue();
    enabled = filename && *filename && parserName && *parserName;
    if (enabled)
    {
        timeShift = this->par("timeShift").doubleValue();
        endTime = this->par("endTime").doubleValue();
        repeatGap = this->par("repeatGap").doubleValue();

        ev << getFullPath() << ".PcapTrafficGenerator::initialize(): "
                <<"file:" << filename
                << ", timeShift:" << timeShift
                << ", endTime:" << endTime
                << ", repeatGap:" << repeatGap
                << endl;

        pcapFile.open(filename);
        pcapFile.setParser(parserName);
        scheduleNextPacket();
    }
}

void PcapTrafficGenerator::handleMessage(cMessage *msg)
{
    if (!msg->isSelfMessage())
        throw cRuntimeError("PcapTrafficGenerator received a non-self message (module=%s)", getFullPath().c_str());

    if (enabled)
    {
        if (msg->isPacket())
        {
            IPv4Datagram* ipd = dynamic_cast<IPv4Datagram*>(msg);
            if (ipd)
            {
                IPv4Address destAddr = ipd->getDestAddress();
                IPv4RoutingDecision *controlInfo = new IPv4RoutingDecision();
                ipd->setControlInfo(controlInfo);
                send(msg, "out");
                msg = NULL;
            }
        }
    }
    else
        EV << "disabled PcapTrafficGenerator received a message (module=" << getFullPath().c_str() << ")\n";

    delete msg;
    scheduleNextPacket();
}

void PcapTrafficGenerator::scheduleNextPacket()
{
    if (!enabled)
        return;

    if (!pcapFile.isOpen())
        return;

    simtime_t curtime = simTime();
    simtime_t pcaptime = curtime - timeShift;
    cMessage* msg = NULL;

    while (!pcapFile.eof())
    {
        msg = (cMessage*)pcapFile.read(pcaptime);
        pcaptime += timeShift;
        if (endTime > SIMTIME_ZERO && pcaptime > endTime)
        {
            delete msg;
            pcapFile.close();
            enabled = false;
            break;
        }
        if (msg && pcaptime >= curtime)
        {
            scheduleAt(pcaptime, msg);
            break;
        }
        if (pcapFile.eof() && repeatGap > SIMTIME_ZERO)
        {
            pcapFile.restart();
            timeShift = pcaptime + repeatGap;
        }
        delete msg;
    }
}
