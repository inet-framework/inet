//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2005 Wei Yang, Ng
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


//  Cleanup and rewrite: Andras Varga, 2004
//  Implementation of IPv6 version: Wei Yang, Ng, 2005

#include "INETDefs.h"

#include "IPv6ErrorHandling.h"
#include "IPv6ControlInfo.h"
#include "IPv6Datagram.h"

Define_Module(IPv6ErrorHandling);

void IPv6ErrorHandling::initialize()
{
}

void IPv6ErrorHandling::handleMessage(cMessage *msg)
{
    ICMPv6Message *icmpv6Msg = check_and_cast<ICMPv6Message *>(msg);
    IPv6Datagram *d = check_and_cast<IPv6Datagram *>(icmpv6Msg->getEncapsulatedPacket());
    int type = (int)icmpv6Msg->getType();
    int code;
    EV << " Type: " << type;
    if (dynamic_cast<ICMPv6DestUnreachableMsg *>(icmpv6Msg))
    {
        ICMPv6DestUnreachableMsg *msg2 = (ICMPv6DestUnreachableMsg *)icmpv6Msg;
        code = msg2->getCode();
        EV << " Code: " << code;
    }
    else if (dynamic_cast<ICMPv6PacketTooBigMsg *>(icmpv6Msg))
    {
        ICMPv6PacketTooBigMsg *msg2 = (ICMPv6PacketTooBigMsg *)icmpv6Msg;
        code = 0;
    }
    else if (dynamic_cast<ICMPv6TimeExceededMsg *>(icmpv6Msg))
    {
        ICMPv6TimeExceededMsg *msg2 = (ICMPv6TimeExceededMsg *)icmpv6Msg;
        code = msg2->getCode();
        EV << " Code: " << code;
    }
    else if (dynamic_cast<ICMPv6ParamProblemMsg *>(icmpv6Msg))
    {
        ICMPv6ParamProblemMsg *msg2 = (ICMPv6ParamProblemMsg *)icmpv6Msg;
        code = msg2->getCode();
        EV << " Code: " << code;
    }

    EV << " Byte length: " << d->getByteLength()
       << " Src: " << d->getSrcAddress()
       << " Dest: " << d->getDestAddress()
       << " Time: " << simTime()
       << "\n";

    if (type == 1)
        displayType1Msg(code);
    else if (type == 2)
        displayType2Msg();
    else if (type == 3)
        displayType3Msg(code);
    else if (type == 4)
        displayType4Msg(code);
    else
        EV << "Unknown Error Type!" << endl;
    delete icmpv6Msg;
}

void IPv6ErrorHandling::displayType1Msg(int code)
{
    EV << "Destination Unreachable: ";
    if (code == 0)
        EV << "no route to destination\n";
    else if (code == 1)
        EV << "communication with destination administratively prohibited\n";
    else if (code == 3)
        EV << "address unreachable\n";
    else if (code == 4)
        EV << "port unreachable\n";
    else
        EV << "Unknown Error Code!\n";
}

void IPv6ErrorHandling::displayType2Msg()
{
    EV << "Packet Too Big" << endl;
    //Code is always 0 and ignored by the receiver.
}

void IPv6ErrorHandling::displayType3Msg(int code)
{
    EV << "Time Exceeded Message: ";
    if (code == 0)
        EV << "hop limit exceeded in transit\n";
    else if (code == 1)
        EV << "fragment reassembly time exceeded\n";
    else
        EV << "Unknown Error Code!\n";
}

void IPv6ErrorHandling::displayType4Msg(int code)
{
    EV << "Parameter Problem Message: ";
    if (code == 0)
        EV << "erroneous header field encountered\n";
    else if (code == 1)
        EV << "unrecognized Next Header type encountered\n";
    else if (code == 2)
        EV << "unrecognized IPv6 option encountered\n";
    else
        EV << "Unknown Error Code!\n";
}
