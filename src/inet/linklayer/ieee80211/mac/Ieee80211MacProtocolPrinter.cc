//
// Copyright (C) 2018 OpenSim Ltd.
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
// @author: Zoltan Bojthe
//

#include "inet/linklayer/ieee80211/mac/Ieee80211MacProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

Register_Protocol_Printer(&Protocol::ieee80211Mac, Ieee80211MacProtocolPrinter);

void Ieee80211MacProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto macHeader = dynamicPtrCast<const Ieee80211MacHeader>(chunk)) {
        context.infoColumn << "WLAN ";
        if (auto oneAddressHeader = dynamicPtrCast<const Ieee80211OneAddressHeader>(chunk))
            context.destinationColumn << oneAddressHeader->getReceiverAddress();
        if (auto twoAddressHeader = dynamicPtrCast<const Ieee80211TwoAddressHeader>(chunk))
            context.sourceColumn << twoAddressHeader->getTransmitterAddress();
        switch (macHeader->getType()) {
            case ST_ASSOCIATIONREQUEST:
                context.infoColumn << " assoc req";    //TODO
                break;

            case ST_ASSOCIATIONRESPONSE:
                context.infoColumn << " assoc resp";    //TODO
                break;

            case ST_REASSOCIATIONREQUEST:
                context.infoColumn << " reassoc req";    //TODO
                break;

            case ST_REASSOCIATIONRESPONSE:
                context.infoColumn << " reassoc resp";    //TODO
                break;

            case ST_PROBEREQUEST:
                context.infoColumn << " probe request";    //TODO
                break;

            case ST_PROBERESPONSE:
                context.infoColumn << " probe response";    //TODO
                break;

            case ST_BEACON:
                context.infoColumn << "beacon";    //TODO
                break;

            case ST_ATIM:
                context.infoColumn << " atim";    //TODO
                break;

            case ST_DISASSOCIATION:
                context.infoColumn << " disassoc";    //TODO
                break;

            case ST_AUTHENTICATION:
                context.infoColumn << " auth";    //TODO
                break;

            case ST_DEAUTHENTICATION:
                context.infoColumn << " deauth";    //TODO
                break;

            case ST_ACTION:
                context.infoColumn << " action";    //TODO
                break;

            case ST_NOACKACTION:
                context.infoColumn << " noackaction";    //TODO
                break;

            case ST_PSPOLL:
                context.infoColumn << " pspoll";    //TODO
            break;

            case ST_RTS: {
                context.infoColumn << "RTS";
                break;
            }
            case ST_CTS:
                context.infoColumn << "CTS" << macHeader->getReceiverAddress();
                break;

            case ST_ACK:
                context.infoColumn << "ACK" << macHeader->getReceiverAddress();
                break;

            case ST_BLOCKACK_REQ:
                context.infoColumn << "BlockAckReq";    //TODO
                break;

            case ST_BLOCKACK:
                context.infoColumn << "BlockAck";    //TODO
                break;

            case ST_DATA:
            case ST_DATA_WITH_QOS:
                context.infoColumn << "DATA";    //TODO
                break;

            case ST_LBMS_REQUEST:
                context.infoColumn << " lbms req";    //TODO
                break;

            case ST_LBMS_REPORT:
                context.infoColumn << " lbms report";    //TODO
                break;

            default:
                context.infoColumn << "Type=" << macHeader->getType();
                break;
        }
    }
    else
        context.infoColumn << "(IEEE 802.11 Mac) " << chunk;
}

} // namespace ieee80211
} // namespace inet

