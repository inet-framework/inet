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

#include "inet/common/INETDefs.h"

#include "inet/networklayer/ipv6/IPv6ErrorHandling.h"
#include "inet/networklayer/ipv6/IPv6Datagram.h"

namespace inet {

Define_Module(IPv6ErrorHandling);

void IPv6ErrorHandling::initialize()
{
}

void IPv6ErrorHandling::handleMessage(cMessage *msg)
{
    ICMPv6Message *icmpv6Msg = check_and_cast<ICMPv6Message *>(msg);
    IPv6Datagram *d = check_and_cast<IPv6Datagram *>(icmpv6Msg->getEncapsulatedPacket());
    int type = (int)icmpv6Msg->getType();

    EV_ERROR << " Type: " << type;

    switch (type) {
        case ICMPv6_DESTINATION_UNREACHABLE: {
            ICMPv6DestUnreachableMsg *msg2 = check_and_cast<ICMPv6DestUnreachableMsg *>(icmpv6Msg);
            int code = msg2->getCode();
            EV_ERROR << " Code: " << code;
            displayType1Msg(code);
            break;
        }

        case ICMPv6_PACKET_TOO_BIG: {
            ICMPv6PacketTooBigMsg *msg2 = check_and_cast<ICMPv6PacketTooBigMsg *>(icmpv6Msg);
            int code = msg2->getCode();
            int mtu = msg2->getMTU();
            EV_ERROR << " Code: " << code << " MTU: " << mtu;
            //Code is always 0 and ignored by the receiver.
            displayType2Msg();
            break;
        }

        case ICMPv6_TIME_EXCEEDED: {
            ICMPv6TimeExceededMsg *msg2 = check_and_cast<ICMPv6TimeExceededMsg *>(icmpv6Msg);
            int code = msg2->getCode();
            EV_ERROR << " Code: " << code;
            displayType3Msg(code);
            break;
        }

        case ICMPv6_PARAMETER_PROBLEM: {
            ICMPv6ParamProblemMsg *msg2 = check_and_cast<ICMPv6ParamProblemMsg *>(icmpv6Msg);
            int code = msg2->getCode();
            EV_ERROR << " Code: " << code;
            displayType4Msg(code);
            break;
        }

        default:
            cEnum *e = cEnum::get("inet::ICMPv6Type");
            const char *str = e->getStringFor(type);
            if (str)
                EV_ERROR << " " << str << endl;
            else
                EV_ERROR << " Unknown Error Type" << endl;
            break;
    }

    EV_DETAIL << "Datagram: Byte length: " << d->getByteLength()
              << " Src: " << d->getSrcAddress()
              << " Dest: " << d->getDestAddress()
              << " Time: " << simTime()
              << endl;

    delete icmpv6Msg;
}

void IPv6ErrorHandling::displayType1Msg(int code)
{
    EV_ERROR << " Destination Unreachable: ";
    switch (code) {
        case NO_ROUTE_TO_DEST:
            EV_ERROR << "no route to destination\n";
            break;

        case COMM_WITH_DEST_PROHIBITED:
            EV_ERROR << "communication with destination administratively prohibited\n";
            break;

        case ADDRESS_UNREACHABLE:
            EV_ERROR << "address unreachable\n";
            break;

        case PORT_UNREACHABLE:
            EV_ERROR << "port unreachable\n";
            break;

        default:
            EV_ERROR << "Unknown Error Code!\n";
            break;
    }
}

void IPv6ErrorHandling::displayType2Msg()
{
    EV_ERROR << " Packet Too Big\n";
}

void IPv6ErrorHandling::displayType3Msg(int code)
{
    EV_ERROR << " Time Exceeded Message: ";
    switch (code) {
        case ND_HOP_LIMIT_EXCEEDED:
            EV_ERROR << "hop limit exceeded in transit\n";
            break;

        case ND_FRAGMENT_REASSEMBLY_TIME:
            EV_ERROR << "fragment reassembly time exceeded\n";
            break;

        default:
            EV_ERROR << "Unknown Error Code!\n";
            break;
    }
}

void IPv6ErrorHandling::displayType4Msg(int code)
{
    EV_ERROR << " Parameter Problem Message: ";
    switch (code) {
        case ERROREOUS_HDR_FIELD:
            EV_ERROR << "erroneous header field encountered\n";
            break;

        case UNRECOGNIZED_NEXT_HDR_TYPE:
            EV_ERROR << "unrecognized Next Header type encountered\n";
            break;

        case UNRECOGNIZED_IPV6_OPTION:
            EV_ERROR << "unrecognized IPv6 option encountered\n";
            break;

        default:
            EV_ERROR << "Unknown Error Code!\n";
            break;
    }
}

} // namespace inet

