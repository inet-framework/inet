//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETDISSECTOR_H
#define __INET_PACKETDISSECTOR_H

#include <functional>
#include <stack>

#include "inet/common/Protocol.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/dissector/ProtocolDissector.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"

namespace inet {

/**
 * This class provides efficient general purpose packet dissection. Packet
 * dissection may be useful for printing packet details, filter packets,
 * finding information in a packet deep down.
 *
 * The supported protocols are provided by a ProtocolDissectorRegistry. The
 * packet dissection algorithm calls the visitChunk() method of the provided
 * PacketDissector::ICallback for each protocol specific chunk found in the
 * packet. The chunks are passed to the method in the order they appear in the
 * packet from left to right using their most specific protocol dependent form.
 */
class INET_API PacketDissector
{
  public:
    class INET_API ICallback {
      public:
        /**
         * True means the packet dissector should recursively process the PDU.
         * When false, start and end notifications and one visit chunk are still generated.
         */
        virtual bool shouldDissectProtocolDataUnit(const Protocol *protocol) = 0;

        /**
         * Notifies about the start of a new protocol data unit (PDU).
         */
        virtual void startProtocolDataUnit(const Protocol *protocol) = 0;

        /**
         * Notifies about the end of the current protocol data unit (PDU).
         */
        virtual void endProtocolDataUnit(const Protocol *protocol) = 0;

        /**
         * Marks the current protocol data unit as incorrect (e.g. bad CRC/FCS, incorrect length field, bit error).
         */
        virtual void markIncorrect() = 0;

        /**
         * Notifies about a new chunk in the current protocol data unit (PDU).
         */
        virtual void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) = 0;
    };

    class INET_API ProtocolDissectorCallback : public ProtocolDissector::ICallback {
      protected:
        const PacketDissector& packetDissector;

      public:
        ProtocolDissectorCallback(const PacketDissector& packetDissector);

        virtual void startProtocolDataUnit(const Protocol *protocol) override;
        virtual void endProtocolDataUnit(const Protocol *protocol) override;
        virtual void markIncorrect() override;
        virtual void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) override;
        virtual void dissectPacket(Packet *packet, const Protocol *protocol) override;
    };

    class INET_API ProtocolDataUnit : public Chunk {
      protected:
        int level;
        bool isCorrect_ = true;
        const Protocol *protocol;
        std::deque<Ptr<const Chunk>> chunks;

      public:
        ProtocolDataUnit(int level, const Protocol *protocol);

        int getLevel() const { return level; }
        bool isCorrect() const { return isCorrect_; }
        const Protocol *getProtocol() const { return protocol; }
        const std::deque<Ptr<const Chunk>>& getChunks() const { return chunks; }

        virtual ChunkType getChunkType() const { throw cRuntimeError("Invalid operation"); }
        virtual b getChunkLength() const;
        virtual const Ptr<Chunk> peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const { throw cRuntimeError("Invalid operation"); }

        void markIncorrect() { isCorrect_ = false; }
        void insert(const Ptr<const Chunk>& chunk) { chunks.push_back(chunk); }
    };

    class INET_API ChunkBuilder : public PacketDissector::ICallback {
      protected:
        Ptr<const Chunk> content;

      public:
        const Ptr<const Chunk> getContent() { return content; }

        virtual bool shouldDissectProtocolDataUnit(const Protocol *protocol) override { return true; }
        virtual void startProtocolDataUnit(const Protocol *protocol) override {}
        virtual void endProtocolDataUnit(const Protocol *protocol) override {}
        virtual void markIncorrect() override {}
        virtual void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) override;
    };

    class INET_API PduTreeBuilder : public PacketDissector::ICallback {
      protected:
        bool isEndProtocolDataUnitCalled = false;
        bool isSimplyEncapsulatedPacket_ = true;

        Ptr<ProtocolDataUnit> topLevelPdu = nullptr;
        Ptr<Chunk> remainingJunk = nullptr;
        std::stack<ProtocolDataUnit *> pduLevels;

      public:
        bool isSimplyEncapsulatedPacket() const { return isSimplyEncapsulatedPacket_; }
        const Ptr<ProtocolDataUnit>& getTopLevelPdu() const { return topLevelPdu; }
        const Ptr<Chunk>& getRemainingJunk() const { return remainingJunk; }

        virtual bool shouldDissectProtocolDataUnit(const Protocol *protocol) override { return true; }
        virtual void startProtocolDataUnit(const Protocol *protocol) override;
        virtual void endProtocolDataUnit(const Protocol *protocol) override;
        virtual void markIncorrect() override;
        virtual void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) override;
    };

  protected:
    const ProtocolDissectorRegistry& protocolDissectorRegistry;
    ICallback& callback;

    void doDissectPacket(Packet *packet, const Protocol *protocol) const;

  public:
    PacketDissector(const ProtocolDissectorRegistry& protocolDissectorRegistry, ICallback& callback);

    /**
     * Dissects the given packet according to the attached PacketProtocolTag.
     */
    void dissectPacket(Packet *packet) const;

    /**
     * Dissects the given packet of the provided protocol. The packet dissection
     * algorithm calls the visitChunk() method of the provided callback for each
     * protocol specific chunk found in the packet.
     */
    void dissectPacket(Packet *packet, const Protocol *protocol, b extraFrontOffset = b(0), b extraBackOffset = b(0)) const;
};

} // namespace

#endif

