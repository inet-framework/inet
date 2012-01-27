//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004-2005 Andras Varga
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


#include "IPvXTrafGen.h"

#include "IPvXAddressResolver.h"
#include "IPv4ControlInfo.h"
#include "IPv6ControlInfo.h"


Define_Module(IPvXTrafSink);


simsignal_t IPvXTrafSink::rcvdPkSignal = SIMSIGNAL_NULL;

void IPvXTrafSink::initialize()
{
    numReceived = 0;
    WATCH(numReceived);
    rcvdPkSignal = registerSignal("rcvdPk");
}

void IPvXTrafSink::handleMessage(cMessage *msg)
{
    processPacket(check_and_cast<cPacket *>(msg));

    if (ev.isGUI())
    {
        char buf[32];
        sprintf(buf, "rcvd: %d pks", numReceived);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void IPvXTrafSink::printPacket(cPacket *msg)
{
    IPvXAddress src, dest;
    int protocol = -1;

    if (dynamic_cast<IPv4ControlInfo *>(msg->getControlInfo()) != NULL)
    {
        IPv4ControlInfo *ctrl = (IPv4ControlInfo *)msg->getControlInfo();
        src = ctrl->getSrcAddr();
        dest = ctrl->getDestAddr();
        protocol = ctrl->getProtocol();
    }
    else if (dynamic_cast<IPv6ControlInfo *>(msg->getControlInfo()) != NULL)
    {
        IPv6ControlInfo *ctrl = (IPv6ControlInfo *)msg->getControlInfo();
        src = ctrl->getSrcAddr();
        dest = ctrl->getDestAddr();
        protocol = ctrl->getProtocol();
    }

    ev  << msg << endl;
    ev  << "Payload length: " << msg->getByteLength() << " bytes" << endl;

    if (protocol != -1)
        ev  << "src: " << src << "  dest: " << dest << "  protocol=" << protocol << "\n";
}

void IPvXTrafSink::processPacket(cPacket *msg)
{
    emit(rcvdPkSignal, msg);
    EV << "Received packet: ";
    printPacket(msg);
    delete msg;
    numReceived++;
}

