// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHAMPDULIMITRADIO_H
#define INET_TEST_MESHAMPDULIMITRADIO_H

class MeshAmpduLimitRadio : public Ieee80211Radio
{
    mutable bool injected = false;
  protected:
    void decapsulate(Packet *packet) const override {
        int scenario = getSimulation()->getSystemModule()->getSubmodule("driver")->par("scenario");
        auto actual = packet->findTag<Ieee80211ModeInd>();
        if (!injected && (scenario == 11 || scenario == 12) && this == meshInterface(1)->getSubmodule("radio")
                && actual && actual->getTxVector().ampdu && std::string(packet->getName()).find("limit-data") != std::string::npos) {
            auto phy = peekIeee80211PhyHeaderAtFront(packet);
            bool vht = actual->getTxVector().format == Ieee80211TxVector::VHT;
            Ieee80211AmpduLayout layout;
            ASSERT(findAmpdu(packet,phy->getChunkLength(),phy->getLengthField(),vht,layout) && layout.contentEnd == B(8191));
            auto psdu = packet->peekDataAt<BytesChunk>(phy->getChunkLength(),phy->getLengthField())->getBytes();
            if (scenario == 11) {
                auto last = layout.members.back();
                auto bytes = packet->peekDataAt<BytesChunk>(phy->getChunkLength()+last.offset,last.length-B(4))->getBytes();
                bytes.push_back(0xab); // One additional valid native payload byte, within this member's bound.
                auto crc = independentMeshFcs(bytes,0,bytes.size());
                for (int shift=0;shift<32;shift+=8) bytes.push_back(uint8_t(crc>>shift));
                auto delimiter = makeShared<Ieee80211MpduSubframeHeader>();
                delimiter->setVht(vht); delimiter->setLength(bytes.size());
                Packet encoded("delimiter",delimiter);
                auto prefix = encoded.peekDataAsBytes()->getBytes();
                if (psdu.size() < last.offset.get<B>()+bytes.size()) psdu.resize(last.offset.get<B>()+bytes.size());
                std::copy(prefix.begin(),prefix.end(),psdu.begin()+last.delimiterOffset.get<B>());
                std::copy(bytes.begin(),bytes.end(),psdu.begin()+last.offset.get<B>());
            }
            else {
                auto spacing = makeShared<Ieee80211MpduSubframeHeader>();
                spacing->setVht(vht); spacing->setLength(0); spacing->setEof(false);
                Packet encoded("spacing",spacing);
                auto bytes = encoded.peekDataAsBytes()->getBytes();
                if (psdu.size() < 8196) psdu.resize(8196,0);
                std::copy(bytes.begin(),bytes.end(),psdu.begin()+8192);
            }
            // VHT consumes modeled EOF padding. The current HT model omits
            // uncoded tail/padding chunks, so this receiver-only reconstruction
            // grows its modeled byte extent while keeping the same raw DATA
            // symbols and actual signal duration (checked below).
            B added = B(psdu.size())-phy->getLengthField();
            ASSERT(added >= B(0));
            auto oldExtent = packet->getDataLength();
            ASSERT(!vht || added == B(0));
            packet->removeDataAt(phy->getChunkLength(),phy->getLengthField());
            packet->insertDataAt(makeShared<BytesChunk>(psdu),phy->getChunkLength());
            auto changed = packet->removeAtFront<Ieee80211PhyHeader>();
            changed->setLengthField(B(psdu.size())); packet->insertAtFront(changed);
            ASSERT(packet->getDataLength() == oldExtent+added);
            Ieee80211AmpduLayout invalid;
            ASSERT(findAmpdu(packet,changed->getChunkLength(),changed->getLengthField(),vht,invalid));
            ASSERT(invalid.contentEnd == B(scenario == 11 ? 8192 : 8196));
            ASSERT(!isAmpduWithinLengthLimit(invalid,B(8191),vht));
            ASSERT(actual->getMode()->getDataMode()->getDuration(vht ? invalid.preEofLength : invalid.psduLength)
                == actual->getMode()->getDataMode()->getDuration(vht ? layout.preEofLength : layout.psduLength));
            for (const auto& item : invalid.members) ASSERT(isAmpduMemberFcsOk(packet,changed->getChunkLength()+item.offset,item.length));
            injected = true;
        }
        Ieee80211Radio::decapsulate(packet);
    }
};
Define_Module(MeshAmpduLimitRadio);

#endif
