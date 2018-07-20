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
#include "inet/linklayer/ieee80211/mac/Ieee80211MacHeaderSerializer.h"

namespace inet {

namespace ieee80211 {

Register_Serializer(Ieee80211MacHeader, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211DataOrMgmtHeader, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211DataHeader, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211MgmtHeader, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211MsduSubframeHeader, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211MpduSubframeHeader, Ieee80211MacHeaderSerializer);

Register_Serializer(Ieee80211AckFrame, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211RtsFrame, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211CtsFrame, Ieee80211MacHeaderSerializer);

Register_Serializer(Ieee80211BasicBlockAckReq, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211CompressedBlockAckReq, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211MultiTidBlockAckReq, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211BasicBlockAck, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211CompressedBlockAck, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211MultiTidBlockAck, Ieee80211MacHeaderSerializer);

Register_Serializer(Ieee80211MacTrailer, Ieee80211MacTrailerSerializer);

void Ieee80211MacHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    if (auto ackFrame = dynamicPtrCast<const Ieee80211AckFrame>(chunk))
    {
        stream.writeByte(0xD4);
        stream.writeByte(0);
        stream.writeUint16Be(ackFrame->getDuration().inUnit(SIMTIME_US));
        stream.writeMacAddress(ackFrame->getReceiverAddress());
    }
    else if (auto rtsFrame = dynamicPtrCast<const Ieee80211RtsFrame>(chunk))
    {
        stream.writeByte(0xB4);
        stream.writeByte(0);
        stream.writeUint16Be(rtsFrame->getDuration().inUnit(SIMTIME_US));
        stream.writeMacAddress(rtsFrame->getReceiverAddress());
        stream.writeMacAddress(rtsFrame->getTransmitterAddress());
    }
    else if (auto ctsFrame = dynamicPtrCast<const Ieee80211CtsFrame>(chunk))
    {
        stream.writeByte(0xC4);
        stream.writeByte(0);
        stream.writeUint16Be(ctsFrame->getDuration().inUnit(SIMTIME_US));
        stream.writeMacAddress(ctsFrame->getReceiverAddress());
    }
    else if (auto dataOrMgmtFrame = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(chunk))
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
        stream.writeMacAddress(dataOrMgmtFrame->getReceiverAddress());
        stream.writeMacAddress(dataOrMgmtFrame->getTransmitterAddress());
        stream.writeMacAddress(dataOrMgmtFrame->getAddress3());
        stream.writeUint16Le(dataOrMgmtFrame->getSequenceNumber() << 4
                | dataOrMgmtFrame->getFragmentNumber());

        if (auto dataFrame = dynamicPtrCast<const Ieee80211DataHeader>(chunk)) {
            if (dataFrame->getFromDS() && dataFrame->getToDS())
                stream.writeMacAddress(dataFrame->getAddress4());

            if (dataOrMgmtFrame->getType() == ST_DATA_WITH_QOS) {
                uint16_t qos =
                        (dataFrame->getTid() & 0x0F) |
                        (dataFrame->getAckPolicy() & 0x03 << 5) |
                        (dataFrame->getAMsduPresent() ? 0x80 : 0x00);
                stream.writeUint16Le(qos);
            }
        }

        if (dynamicPtrCast<const Ieee80211ActionFrame>(chunk))
        {
            //type = ST_ACTION;
            // 1    Action
            // Last One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
        }
    }
    else if (auto msduSubframe = dynamicPtrCast<const Ieee80211MsduSubframeHeader>(chunk))
    {
        stream.writeMacAddress(msduSubframe->getDa());
        stream.writeMacAddress(msduSubframe->getSa());
        stream.writeUint16Be(msduSubframe->getLength());
    }
    else if (auto mpduSubframe = dynamicPtrCast<const Ieee80211MpduSubframeHeader>(chunk))
    {
        stream.writeUint4(0);
        stream.writeUint4(mpduSubframe->getLength() >> 8);
        stream.writeUint8(mpduSubframe->getLength() & 0xFF);
        stream.writeByte(0);
        stream.writeByte(0x4E);
    }
    else
        throw cRuntimeError("Ieee80211Serializer: cannot serialize the frame");
}

void Ieee80211MacHeaderSerializer::parseDataOrMgmtFrame(MemoryInputStream &stream, const Ptr<Ieee80211DataOrMgmtHeader> frame, Ieee80211FrameType type, uint8_t fc1) const
{
    frame->setType(type);
    frame->setToDS(fc1 & 0x1);
    frame->setFromDS(fc1 & 0x2);
    frame->setMoreFragments(fc1 & 0x4);
    frame->setRetry(fc1 & 0x8);
    frame->setDuration(SimTime(stream.readUint16Be() / 1000.0)); // i_dur
    frame->setReceiverAddress(stream.readMacAddress());
    frame->setTransmitterAddress(stream.readMacAddress());
    frame->setAddress3(stream.readMacAddress());
    uint16_t seq = stream.readUint16Le();   // i_seq
    frame->setSequenceNumber(seq >> 4);
    frame->setFragmentNumber(seq & 0x0F);

    if ((type == ST_DATA || type == ST_DATA_WITH_QOS) && frame->getFromDS() && frame->getToDS())
        dynamicPtrCast<Ieee80211DataHeader>(frame)->setAddress4(stream.readMacAddress());
    if (type == ST_DATA_WITH_QOS) {
        auto dataHeader = dynamicPtrCast<Ieee80211DataHeader>(frame);
        uint16_t qos = stream.readUint16Le();
        dataHeader->setTid(qos & 0xF);
        dataHeader->setAckPolicy((inet::ieee80211::AckPolicy)((qos >> 5) & 0x03));
        dataHeader->setAMsduPresent(qos & 0x80);
    }
}

const Ptr<Chunk> Ieee80211MacHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    uint8_t type = stream.readByte();
    uint8_t fc_1 = stream.readByte();  (void)fc_1; // fc_1
    if (!(type & 0x30)) {
        auto managementHeader = makeShared<Ieee80211MgmtHeader>();
        parseDataOrMgmtFrame(stream, managementHeader, type == 0x08 ? ST_DATA : ST_DATA_WITH_QOS, fc_1);
        return managementHeader;
    }
    switch(type)
    {
        case 0xD4: // ST_ACK    //TODO ((ST_ACK & 0x0F) << 4) | ((ST_ACK & 0x30) >> 2)
        {
            auto ackFrame = makeShared<Ieee80211AckFrame>();
            ackFrame->setType(ST_ACK);
            ackFrame->setToDS(false);
            ackFrame->setFromDS(false);
            ackFrame->setRetry(false);
            ackFrame->setMoreFragments(false);
            ackFrame->setDuration(SimTime((double)stream.readUint16Be()/1000.0));    //i_dur
            ackFrame->setReceiverAddress(stream.readMacAddress());
            return ackFrame;
        }
        case 0xB4: // ST_RTS
        {
            auto rtsFrame = makeShared<Ieee80211RtsFrame>();
            rtsFrame->setType(ST_RTS);
            rtsFrame->setToDS(false);
            rtsFrame->setFromDS(false);
            rtsFrame->setRetry(false);
            rtsFrame->setMoreFragments(false);
            rtsFrame->setDuration(SimTime(stream.readUint16Be(), SIMTIME_US));    //i_dur
            rtsFrame->setReceiverAddress(stream.readMacAddress());
            rtsFrame->setTransmitterAddress(stream.readMacAddress());
            return rtsFrame;
        }
        case 0xC4: // ST_CTS
        {
            auto ctsFrame = makeShared<Ieee80211CtsFrame>();
            ctsFrame->setType(ST_CTS);
            ctsFrame->setToDS(false);
            ctsFrame->setFromDS(false);
            ctsFrame->setRetry(false);
            ctsFrame->setMoreFragments(false);
            ctsFrame->setDuration(SimTime(stream.readUint16Be(),SIMTIME_US));    //i_dur
            ctsFrame->setReceiverAddress(stream.readMacAddress());
            return ctsFrame;
        }
        case 0x08: // ST_DATA
        case 0x88: // ST_DATA_WITH_QOS
        {
            auto dataHeader = makeShared<Ieee80211DataHeader>();
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

void Ieee80211MacTrailerSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& macTrailer = dynamicPtrCast<const Ieee80211MacTrailer>(chunk);
    auto fcsMode = macTrailer->getFcsMode();
    if (fcsMode != FCS_COMPUTED)
        throw cRuntimeError("Cannot serialize Ieee80211FcsTrailer without properly computed FCS, try changing the value of the fcsMode parameter (e.g. in the Ieee80211Mac module)");
    stream.writeUint32Be(macTrailer->getFcs());
}

const Ptr<Chunk> Ieee80211MacTrailerSerializer::deserialize(MemoryInputStream& stream) const
{
    auto macTrailer = makeShared<Ieee80211MacTrailer>();
    auto fcs = stream.readUint32Be();
    macTrailer->setFcs(fcs);
    macTrailer->setFcsMode(FCS_COMPUTED);
    return macTrailer;
}

} // namespace ieee80211

} // namespace inet

