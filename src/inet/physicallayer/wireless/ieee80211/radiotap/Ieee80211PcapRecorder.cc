// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/physicallayer/wireless/ieee80211/radiotap/Ieee80211PcapRecorder.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/ieee80211/mac/aggregation/Ieee80211Ampdu.h"
#include "inet/linklayer/ieee80211/mac/contract/Ieee80211AmpduObservationTag_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet::physicallayer {

Define_Module(Ieee80211PcapRecorder);

void Ieee80211PcapRecorder::recordPacket(const cPacket *message, Direction direction, cComponent *source)
{
    auto packet = dynamic_cast<const Packet *>(message);
    if (!packet) {
        PcapRecorder::recordPacket(message,direction,source);
        return;
    }
    auto request = packet->findTag<Ieee80211ModeReq>();
    auto indication = packet->findTag<Ieee80211ModeInd>();
    auto vector = indication ? indication->getTxVector() : request ? request->getTxVector() : Ieee80211TxVector();
    auto protocol = packet->findTag<PacketProtocolTag>();
    bool physical = protocol && (protocol->getProtocol() == &Protocol::ieee80211HtPhy || protocol->getProtocol() == &Protocol::ieee80211VhtPhy);
    if (!vector.ampdu || !physical) {
        PcapRecorder::recordPacket(message,direction,source);
        return;
    }
    constexpr int PEEK_FLAGS = Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE | Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED;
    auto header = Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet,b(-1),PEEK_FLAGS);
    ieee80211::Ieee80211AmpduLayout layout;
    if (!header->isCorrect() || !header->isComplete() || !header->isProperlyRepresented()
            || !ieee80211::findAmpdu(packet,header->getChunkLength(),header->getLengthField(),vector.format == Ieee80211TxVector::VHT,layout))
        return; // Atomic validation: never publish a structurally valid prefix.
    uint32_t reference = nextAmpduReference++;
    for (size_t i=0;i<layout.members.size();i++) {
        const auto& span = layout.members[i];
        b offset = header->getChunkLength()+span.offset;
        std::unique_ptr<Packet> member(new Packet(packet->getName(),packet->peekDataAt(offset,span.length)));
        member->copyTags(*packet);
        member->copyRegionTags(*packet,packet->getFrontOffset()+offset,b(0),span.length);
        member->setBitError(packet->hasBitError() || !ieee80211::isAmpduMemberFcsOk(packet,offset,span.length));
        member->addTagIfAbsent<PacketProtocolTag>()->set(&Protocol::ieee80211Mac);
        auto observation = member->addTagIfAbsent<ieee80211::Ieee80211AmpduObservationTag>();
        observation->setReference(reference);
        observation->setMemberIndex(i);
        observation->setMemberCount(layout.members.size());
        observation->setDelimiterCrc(packet->peekDataAt<BytesChunk>(header->getChunkLength()+span.delimiterOffset+B(2),B(1))->getBytes()[0]);
        PcapRecorder::recordPacket(member.get(),direction,source);
    }
}

} // namespace inet::physicallayer
