//
// Copyright (C) 2014 OpenSim Ltd.
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

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/common/serializer/ieee80211/Ieee80211MacHeaderSerializer.h"

namespace inet {

namespace serializer {

Register_Serializer(Ieee80211Frame, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211DataOrMgmtHeader, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211DataHeader, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211MgmtHeader, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211MsduSubframeHeader, Ieee80211MacHeaderSerializer);

Register_Serializer(Ieee80211ACKFrame, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211RTSFrame, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211CTSFrame, Ieee80211MacHeaderSerializer);

Register_Serializer(Ieee80211BasicBlockAckReq, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211CompressedBlockAckReq, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211MultiTidBlockAckReq, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211BasicBlockAck, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211CompressedBlockAck, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211MultiTidBlockAck, Ieee80211MacHeaderSerializer);

Register_Serializer(Ieee80211MacTrailer, Ieee80211MacTrailerSerializer);

void Ieee80211MacHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<Chunk>& chunk) const
{
    if (auto ackFrame = std::dynamic_pointer_cast<Ieee80211ACKFrame>(chunk))
    {
        stream.writeByte(0xD4);
        stream.writeByte(0);
        stream.writeUint16Be(ackFrame->getDuration().inUnit(SIMTIME_US));
        stream.writeMACAddress(ackFrame->getReceiverAddress());
    }
    else if (auto rtsFrame = std::dynamic_pointer_cast<Ieee80211RTSFrame>(chunk))
    {
        stream.writeByte(0xB4);
        stream.writeByte(0);
        stream.writeUint16Be(rtsFrame->getDuration().inUnit(SIMTIME_US));
        stream.writeMACAddress(rtsFrame->getReceiverAddress());
        stream.writeMACAddress(rtsFrame->getTransmitterAddress());
    }
    else if (auto ctsFrame = std::dynamic_pointer_cast<Ieee80211CTSFrame>(chunk))
    {
        stream.writeByte(0xC4);
        stream.writeByte(0);
        stream.writeUint16Be(ctsFrame->getDuration().inUnit(SIMTIME_US));
        stream.writeMACAddress(ctsFrame->getReceiverAddress());
    }
    else if (auto dataOrMgmtFrame = std::dynamic_pointer_cast<Ieee80211DataOrMgmtHeader>(chunk))
    {
        uint8_t type = dataOrMgmtFrame->getType();
        stream.writeByte(((type & 0x0F) << 4) | ((type & 0x30) >> 2));  //without Qos=0x08, with Qos=0x88
        uint8_t fc1;
        // TODO: Order, Protected Frame, MoreData, Power Mgt
        fc1 = (dataOrMgmtFrame->getRetry() ? 8 : 0)
                | (dataOrMgmtFrame->getMoreFragments() ? 4 : 0)
                | (dataOrMgmtFrame->getFromDS() ? 2 : 0)
                | (dataOrMgmtFrame->getToDS() ? 1 : 0);
        stream.writeByte(fc1);
        stream.writeUint16Be(dataOrMgmtFrame->getDuration().inUnit(SIMTIME_US));
        stream.writeMACAddress(dataOrMgmtFrame->getReceiverAddress());
        stream.writeMACAddress(dataOrMgmtFrame->getTransmitterAddress());
        stream.writeMACAddress(dataOrMgmtFrame->getAddress3());
        stream.writeUint16Le(dataOrMgmtFrame->getSequenceNumber() << 4
                | dataOrMgmtFrame->getFragmentNumber());

        if (auto dataFrame = std::dynamic_pointer_cast<Ieee80211DataHeader>(chunk)) {
            if (dataFrame->getFromDS() && dataFrame->getToDS())
                stream.writeMACAddress(dataFrame->getAddress4());

            if (dataOrMgmtFrame->getType() == ST_DATA_WITH_QOS)
                stream.writeUint16Le(dataFrame->getQos() | dataFrame->getTid() | (dataFrame->getAMsduPresent() ? 0x0080 : 0x0000));
        }

        if (std::dynamic_pointer_cast<Ieee80211ActionFrame>(chunk))
        {
            //type = ST_ACTION;
            // 1    Action
            // Last One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
        }
    }
    else if (auto msduSubframe = std::dynamic_pointer_cast<Ieee80211MsduSubframeHeader>(chunk))
    {
        stream.writeMACAddress(msduSubframe->getDa());
        stream.writeMACAddress(msduSubframe->getSa());
        stream.writeUint16Be(msduSubframe->getLength());
    }
    else
        throw cRuntimeError("Ieee80211Serializer: cannot serialize the frame");
}

void Ieee80211MacHeaderSerializer::parseDataOrMgmtFrame(MemoryInputStream &stream, Ptr<Ieee80211DataOrMgmtHeader> frame, short type, uint8_t fc1) const
{
    frame->setType(type);
    frame->setToDS(fc1 & 0x1);
    frame->setFromDS(fc1 & 0x2);
    frame->setMoreFragments(fc1 & 0x4);
    frame->setRetry(fc1 & 0x8);
    frame->setDuration(SimTime(stream.readUint16Be() / 1000.0)); // i_dur
    frame->setReceiverAddress(stream.readMACAddress());
    frame->setTransmitterAddress(stream.readMACAddress());
    frame->setAddress3(stream.readMACAddress());
    uint16_t seq = stream.readUint16Le();   // i_seq
    frame->setSequenceNumber(seq >> 4);
    frame->setFragmentNumber(seq & 0x0F);

    if ((type == ST_DATA || type == ST_DATA_WITH_QOS) && frame->getFromDS() && frame->getToDS())
        std::dynamic_pointer_cast<Ieee80211DataHeader>(frame)->setAddress4(stream.readMACAddress());
    if (type == ST_DATA_WITH_QOS)
        std::dynamic_pointer_cast<Ieee80211DataHeader>(frame)->setQos(stream.readUint16Le());
}

Ptr<Chunk> Ieee80211MacHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    uint8_t type = stream.readByte();
    uint8_t fc_1 = stream.readByte();  (void)fc_1; // fc_1
    if (!(type & 0x30)) {
        auto managementHeader = std::make_shared<Ieee80211MgmtHeader>();
        parseDataOrMgmtFrame(stream, managementHeader, type == 0x08 ? ST_DATA : ST_DATA_WITH_QOS, fc_1);
        return managementHeader;
    }
    switch(type)
    {
        case 0xD4: // ST_ACK    //TODO ((ST_ACK & 0x0F) << 4) | ((ST_ACK & 0x30) >> 2)
        {
            auto ackFrame = std::make_shared<Ieee80211ACKFrame>();
            ackFrame->setType(ST_ACK);
            ackFrame->setToDS(false);
            ackFrame->setFromDS(false);
            ackFrame->setRetry(false);
            ackFrame->setMoreFragments(false);
            ackFrame->setDuration(SimTime((double)stream.readUint16Be()/1000.0));    //i_dur
            ackFrame->setReceiverAddress(stream.readMACAddress());
            return ackFrame;
        }
        case 0xB4: // ST_RTS
        {
            auto rtsFrame = std::make_shared<Ieee80211RTSFrame>();
            rtsFrame->setType(ST_RTS);
            rtsFrame->setToDS(false);
            rtsFrame->setFromDS(false);
            rtsFrame->setRetry(false);
            rtsFrame->setMoreFragments(false);
            rtsFrame->setDuration(SimTime(stream.readUint16Be(), SIMTIME_US));    //i_dur
            rtsFrame->setReceiverAddress(stream.readMACAddress());
            rtsFrame->setTransmitterAddress(stream.readMACAddress());
            return rtsFrame;
        }
        case 0xC4: // ST_CTS
        {
            auto ctsFrame = std::make_shared<Ieee80211CTSFrame>();
            ctsFrame->setType(ST_CTS);
            ctsFrame->setToDS(false);
            ctsFrame->setFromDS(false);
            ctsFrame->setRetry(false);
            ctsFrame->setMoreFragments(false);
            ctsFrame->setDuration(SimTime(stream.readUint16Be(),SIMTIME_US));    //i_dur
            ctsFrame->setReceiverAddress(stream.readMACAddress());
            return ctsFrame;
        }
        case 0x08: // ST_DATA
        case 0x88: // ST_DATA_WITH_QOS
        {
            auto dataHeader = std::make_shared<Ieee80211DataHeader>();
            parseDataOrMgmtFrame(stream, dataHeader, type == 0x08 ? ST_DATA : ST_DATA_WITH_QOS, fc_1);
            return dataHeader;
        }

        case 0xD0: // type = ST_ACTION
        {
            // Ieee80211ActionFrame
            return nullptr;
        }

        default:
            throw cRuntimeError("Cannot deserialize frame");
    }
}

void Ieee80211MacTrailerSerializer::serialize(MemoryOutputStream& stream, const Ptr<Chunk>& chunk) const
{
    const auto& macTrailer = std::dynamic_pointer_cast<Ieee80211MacTrailer>(chunk);
    auto fcsMode = macTrailer->getFcsMode();
//    if (fcsMode != FCS_COMPUTED)
//        throw cRuntimeError("Cannot serialize Ieee80211FcsTrailer without properly computed FCS, try changing the value of the fcsMode parameter (e.g. in the Ieee80211Mac module)");
    stream.writeUint32Be(macTrailer->getFcs());
}

Ptr<Chunk> Ieee80211MacTrailerSerializer::deserialize(MemoryInputStream& stream) const
{
    auto macTrailer = std::make_shared<Ieee80211MacTrailer>();
    auto fcs = stream.readUint32Be();
    macTrailer->setFcs(fcs);
    macTrailer->setFcsMode(FCS_COMPUTED);
    return macTrailer;
}

} // namespace serializer

} // namespace inet

