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

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211MacProtocolPrinter.h"

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
                context.typeColumn << "AssocReq";    //TODO
                break;

            case ST_ASSOCIATIONRESPONSE:
                context.typeColumn << "AssocResp";    //TODO
                break;

            case ST_REASSOCIATIONREQUEST:
                context.typeColumn << "ReassocReq";    //TODO
                break;

            case ST_REASSOCIATIONRESPONSE:
                context.typeColumn << "ReassocResp";    //TODO
                break;

            case ST_PROBEREQUEST:
                context.typeColumn << "ProbeRequest";    //TODO
                break;

            case ST_PROBERESPONSE:
                context.typeColumn << "ProbeResponse";    //TODO
                break;

            case ST_BEACON:
                context.typeColumn << "Beacon";    //TODO
                break;

            case ST_ATIM:
                context.typeColumn << "Atim";    //TODO
                break;

            case ST_DISASSOCIATION:
                context.typeColumn << "Disassoc";    //TODO
                break;

            case ST_AUTHENTICATION:
                context.typeColumn << "Auth";    //TODO
                break;

            case ST_DEAUTHENTICATION:
                context.typeColumn << "Deauth";    //TODO
                break;

            case ST_ACTION:
                context.typeColumn << "Action";    //TODO
                break;

            case ST_NOACKACTION:
                context.typeColumn << "Noackaction";    //TODO
                break;

            case ST_PSPOLL:
                context.typeColumn << "Pspoll";    //TODO
            break;

            case ST_RTS: {
                context.typeColumn << "RTS";
                break;
            }
            case ST_CTS:
                context.typeColumn << "CTS";
                context.infoColumn << macHeader->getReceiverAddress();
                break;

            case ST_ACK:
                context.typeColumn << "ACK";
                context.infoColumn << macHeader->getReceiverAddress();
                break;

            case ST_BLOCKACK_REQ:
                context.typeColumn << "BlockAckReq";    //TODO
                break;

            case ST_BLOCKACK:
                context.typeColumn << "BlockAck";    //TODO
                break;

            case ST_DATA:
            case ST_DATA_WITH_QOS:
                context.typeColumn << "DATA";    //TODO
                break;

            case ST_LBMS_REQUEST:
                context.typeColumn << "LbmsReq";    //TODO
                break;

            case ST_LBMS_REPORT:
                context.typeColumn << "LbmsReport";    //TODO
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

