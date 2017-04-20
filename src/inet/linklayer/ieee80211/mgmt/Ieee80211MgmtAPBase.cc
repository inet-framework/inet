//
// Copyright (C) 2006 Andras Varga
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

#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtAPBase.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/EtherTypeTag_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/MACAddressTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include <string.h>

#ifdef WITH_ETHERNET
#include "inet/linklayer/ethernet/EtherEncap.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#endif // ifdef WITH_ETHERNET


namespace inet {

namespace ieee80211 {

void Ieee80211MgmtAPBase::initialize(int stage)
{
    Ieee80211MgmtBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        isConnectedToHL = gate("upperLayerOut")->getPathEndGate()->isConnected();
        const char *encDec = par("encapDecap").stringValue();
        if (!strcmp(encDec, "true"))
            encapDecap = ENCAP_DECAP_TRUE;
        else if (!strcmp(encDec, "false"))
            encapDecap = ENCAP_DECAP_FALSE;
        else if (!strcmp(encDec, "eth"))
            encapDecap = ENCAP_DECAP_ETH;
        else
            throw cRuntimeError("Unknown encapDecap parameter value: '%s'! Must be 'true','false' or 'eth'.", encDec);

        WATCH(isConnectedToHL);
    }
}

void Ieee80211MgmtAPBase::distributeReceivedDataFrame(Packet *packet)
{
    packet->removePoppedChunks();
    const auto& header = packet->removeHeader<Ieee80211DataFrame>();

    // adjust toDS/fromDS bits, and shuffle addresses
    header->setToDS(false);
    header->setFromDS(true);

    // move destination address to address1 (receiver address),
    // and fill address3 with original source address;
    // sender address (address2) will be filled in by MAC
    ASSERT(!header->getAddress3().isUnspecified());
    header->setReceiverAddress(header->getAddress3());
    header->setAddress3(header->getTransmitterAddress());

    packet->insertHeader(header);

    sendDown(packet);
}

void Ieee80211MgmtAPBase::sendToUpperLayer(Packet *packet)
{
    if (!isConnectedToHL) {
        delete packet;
        return;
    }
    const auto& frame = packet->peekHeader<Ieee80211DataFrame>();
    switch (encapDecap) {
        case ENCAP_DECAP_ETH:
#ifdef WITH_ETHERNET
            convertToEtherFrame(packet, frame);
#else // ifdef WITH_ETHERNET
            throw cRuntimeError("INET compiled without ETHERNET feature, but the 'encapDecap' parameter is set to 'eth'!");
#endif // ifdef WITH_ETHERNET
            break;

        case ENCAP_DECAP_TRUE: {
            auto macAddressInd = packet->ensureTag<MacAddressInd>();
            macAddressInd->setSrcAddress(frame->getTransmitterAddress());
            macAddressInd->setDestAddress(frame->getAddress3());
            int tid = frame->getTid();
            if (tid < 8)
                packet->ensureTag<UserPriorityInd>()->setUserPriority(tid); // TID values 0..7 are UP
            const Ptr<Ieee80211DataFrameWithSNAP>& frameWithSNAP = std::dynamic_pointer_cast<Ieee80211DataFrameWithSNAP>(frame);
            if (frameWithSNAP) {
                int etherType = frameWithSNAP->getEtherType();
                packet->ensureTag<EtherTypeInd>()->setEtherType(etherType);
                packet->ensureTag<DispatchProtocolReq>()->setProtocol(ProtocolGroup::ethertype.getProtocol(etherType));
                packet->ensureTag<PacketProtocolTag>()->setProtocol(ProtocolGroup::ethertype.getProtocol(etherType));
            }
        }
        break;

        case ENCAP_DECAP_FALSE:
            break;

        default:
            throw cRuntimeError("Unknown encapDecap value: %d", encapDecap);
            break;
    }
    sendUp(packet);
}

void Ieee80211MgmtAPBase::convertToEtherFrame(Packet *packet, const Ptr<Ieee80211DataFrame>& frame_)
{
#ifdef WITH_ETHERNET
    const Ptr<Ieee80211DataFrameWithSNAP>& frame = std::dynamic_pointer_cast<Ieee80211DataFrameWithSNAP>(frame_);

    // create a matching ethernet frame
    const auto& ethframe = std::make_shared<EthernetIIFrame>();    //TODO option to use EtherFrameWithSNAP instead
    ethframe->setDest(frame->getAddress3());
    ethframe->setSrc(frame->getTransmitterAddress());
    ethframe->setEtherType(frame->getEtherType());
    ethframe->setChunkLength(byte(ETHER_MAC_FRAME_BYTES - 4)); // subtract FCS

    // encapsulate the payload in there
    packet->removeHeader<Ieee80211DataFrame>();
    ethframe->markImmutable();
    packet->pushHeader(ethframe);
    EtherEncap::addPaddingAndFcs(packet);

    packet->ensureTag<DispatchProtocolReq>()->setProtocol(&Protocol::ethernet);
    packet->ensureTag<PacketProtocolTag>()->setProtocol(&Protocol::ethernet);
#else // ifdef WITH_ETHERNET
    throw cRuntimeError("INET compiled without ETHERNET feature!");
#endif // ifdef WITH_ETHERNET
}

const Ptr<Ieee80211DataFrame>& Ieee80211MgmtAPBase::convertFromEtherFrame(Packet *packet)
{
#ifdef WITH_ETHERNET
    auto ethframe = EtherEncap::decapsulate(packet);       // do not use const auto& : removePoppedChunks() delete it from packet
    // create new frame
    const Ptr<Ieee80211DataFrameWithSNAP>& frame = std::make_shared<Ieee80211DataFrameWithSNAP>();
    frame->setFromDS(true);
    packet->removePoppedChunks();

    // copy addresses from ethernet frame (transmitter addr will be set to our addr by MAC)
    frame->setReceiverAddress(ethframe->getDest());
    frame->setAddress3(ethframe->getSrc());

    // copy EtherType from original frame
    if (const auto& eth2frame = std::dynamic_pointer_cast<EthernetIIFrame>(ethframe))
        frame->setEtherType(eth2frame->getEtherType());
    else if (const auto& snapframe = std::dynamic_pointer_cast<EtherFrameWithSNAP>(ethframe))
        frame->setEtherType(snapframe->getLocalcode());
    else
        throw cRuntimeError("Unaccepted EtherFrame type: %s, contains no EtherType", ethframe->getClassName());

    // encapsulate payload
    packet->insertHeader(frame);

    // done
    return frame;
#else // ifdef WITH_ETHERNET
    throw cRuntimeError("INET compiled without ETHERNET feature!");
#endif // ifdef WITH_ETHERNET
}

void Ieee80211MgmtAPBase::encapsulate(Packet *msg)
{
    switch (encapDecap) {
        case ENCAP_DECAP_ETH:
#ifdef WITH_ETHERNET
            convertFromEtherFrame(check_and_cast<Packet *>(msg));
#else // ifdef WITH_ETHERNET
            throw cRuntimeError("INET compiled without ETHERNET feature, but the 'encapDecap' parameter is set to 'eth'!");
#endif // ifdef WITH_ETHERNET
            break;

        case ENCAP_DECAP_TRUE: {
            const Ptr<Ieee80211DataFrameWithSNAP>& frame = std::make_shared<Ieee80211DataFrameWithSNAP>();
            frame->setFromDS(true);

            // copy addresses from ethernet frame (transmitter addr will be set to our addr by MAC)
            frame->setAddress3(msg->getMandatoryTag<MacAddressReq>()->getSrcAddress());
            frame->setReceiverAddress(msg->getMandatoryTag<MacAddressReq>()->getDestAddress());
            frame->setEtherType(msg->getMandatoryTag<EtherTypeReq>()->getEtherType());
            auto userPriorityReq = msg->getTag<UserPriorityReq>();
            if (userPriorityReq != nullptr) {
                // make it a QoS frame, and set TID
                frame->setType(ST_DATA_WITH_QOS);
                frame->setChunkLength(frame->getChunkLength() + bit(QOSCONTROL_BITS));
                frame->setTid(userPriorityReq->getUserPriority());
            }

            // encapsulate payload
            msg->insertHeader(frame);
        }
        break;

        case ENCAP_DECAP_FALSE:
            break;

        default:
            throw cRuntimeError("Unknown encapDecap value: %d", encapDecap);
            break;
    }
}

} // namespace ieee80211

} // namespace inet

