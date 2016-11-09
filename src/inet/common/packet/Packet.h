//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __INET_PACKET_H_
#define __INET_PACKET_H_

#include "inet/common/packet/SequenceChunk.h"

namespace inet {

/**
 * This class represents network packets, datagrams, frames and other kinds
 * data used by communication protocols.
 */
class Packet : public cPacket
{
  protected:
    // TODO: data could be std::shared_ptr<Chunk> simply, e.g. allowing for plain ByteLengthChunk or ByteArrayChunk if that's what we need
    std::shared_ptr<SequenceChunk> data;
    SequenceChunk::ForwardIterator headerIterator;
    SequenceChunk::BackwardIterator trailerIterator;

  public:
    explicit Packet(const char *name = nullptr, short kind = 0);
    Packet(const Packet& other);

    virtual Packet *dup() const override { return new Packet(*this); }

    /** @name Mutability related functions */
    //@{
    bool isImmutable() const { return data->isImmutable(); }
    bool isMutable() const { return !data->isImmutable(); }
    void assertMutable() const { data->assertMutable(); }
    void assertImmutable() const { data->assertImmutable(); }
    void makeImmutable() { data->makeImmutable(); }
    //@}

    /** @name Header querying related functions */
    //@{
    /**
     * Returns the current header iterator position measured in bytes from the
     * beginning of the packet. The returned value is in the range [0, getByteLength()].
     */
    int64_t getHeaderPosition() const { return headerIterator.getPosition(); }

    /**
     * Changes the current header iterator position measured in bytes from the
     * beginning of the packet. The value must be in the range [0, getByteLength()].
     */
    void setHeaderPosition(int64_t offset) { headerIterator.seek(offset); }

    /**
     * Returns the total byte length of the packet headers processed so far.
     */
    int64_t getHeaderSize() const { return headerIterator.getPosition(); }

    std::shared_ptr<Chunk> peekHeader(int64_t byteLength = -1) const;

    std::shared_ptr<Chunk> peekHeaderAt(int64_t byteOffset, int64_t byteLength) const;

    template <typename T>
    bool hasHeader(int64_t byteLength = -1) const {
        return peekHeader<T>(byteLength) != nullptr;
    }

    template <typename T>
    std::shared_ptr<T> peekHeader(int64_t byteLength = -1) const {
        return data->peek<T>(headerIterator, byteLength);
    }

    template <typename T>
    std::shared_ptr<T> peekHeaderAt(int64_t byteOffset, int64_t byteLength = -1) const {
        return data->peek<T>(SequenceChunk::ForwardIterator(data, -1, byteOffset), byteLength);
    }

    template <typename T>
    std::shared_ptr<T> popHeader(int64_t byteLength = -1) {
        const auto& chunk = peekHeader<T>(byteLength);
        if (chunk != nullptr)
            headerIterator.move(chunk->getByteLength());
        return chunk;
    }
    //@}

    /** @name Trailer querying related functions */
    //@{
    /**
     * Returns the current trailer iterator position measured in bytes from the
     * end of the packet. The returned value is in the range [0, getByteLength()].
     */
    int64_t getTrailerPosition() const { return trailerIterator.getPosition(); }

    /**
     * Changes the current trailer iterator position measured in bytes from the
     * end of the packet. The value must be in the range [0, getByteLength()].
     */
    void setTrailerPosition(int64_t offset) { trailerIterator.seek(offset); }

    /**
     * Returns the total byte length of the packet trailers processed so far.
     */
    int64_t getTrailerSize() const { return trailerIterator.getPosition(); }

    std::shared_ptr<Chunk> peekTrailer(int64_t byteLength = -1) const;

    std::shared_ptr<Chunk> peekTrailerAt(int64_t byteOffset, int64_t byteLength) const;

    template <typename T>
    bool hasTrailer(int64_t byteLength = -1) const {
        return peekTrailer<T>(byteLength) != nullptr;
    }

    template <typename T>
    std::shared_ptr<T> peekTrailer(int64_t byteLength = -1) const {
        return data->peek<T>(trailerIterator, byteLength);
    }

    template <typename T>
    std::shared_ptr<T> peekTrailerAt(int64_t byteOffset, int64_t byteLength = -1) const {
        return data->peek<T>(SequenceChunk::BackwardIterator(data, -1, byteOffset), byteLength);
    }

    template <typename T>
    std::shared_ptr<T> popTrailer(int64_t byteLength = -1) {
        auto& chunk = peekTrailer<T>(byteLength);
        if (chunk != nullptr)
            trailerIterator.move(-chunk->getByteLength());
        return chunk;
    }
    //@}

    /** @name Data querying related functions */
    //@{
    /**
     * Returns the current data position measured in bytes from the beginning
     * of the packet. The returned value is in the range [0, getByteLength()].
     */
    int64_t getDataPosition() const { return headerIterator.getPosition(); }

    /**
     * Returns the current data size measured in bytes. The returned value is
     * in the range [0, getByteLength()].
     */
    int64_t getDataSize() const { return getByteLength() - headerIterator.getPosition() - trailerIterator.getPosition(); }

    std::shared_ptr<Chunk> peekData(int64_t byteLength = -1) const;

    std::shared_ptr<Chunk> peekDataAt(int64_t byteOffset = 0, int64_t byteLength = -1) const;

    template <typename T>
    bool hasData(int64_t byteLength = -1) const {
        return peekData<T>(byteLength) != nullptr;
    }

    template <typename T>
    std::shared_ptr<T> peekData(int64_t byteLength = -1) const {
        return data->peek<T>(SequenceChunk::ForwardIterator(data, -1, getDataPosition()), byteLength);
    }

    template <typename T>
    std::shared_ptr<T> peekDataAt(int64_t byteOffset = 0, int64_t byteLength = -1) const {
        return data->peek<T>(SequenceChunk::ForwardIterator(data, -1, byteOffset), byteLength);
    }
    //@}

    /** @name Filling with data related functions */
    //@{
    void prepend(const std::shared_ptr<Chunk>& chunk, bool flatten = true);
    void prepend(Packet* packet, bool flatten = true);

    void append(const std::shared_ptr<Chunk>& chunk, bool flatten = true);
    void append(Packet* packet, bool flatten = true);
    //@}

    virtual int64_t getBitLength() const override { return data->getByteLength() << 3; }

    virtual std::string str() const override { return data->str(); }

  protected:
    int getNumChunks() const;
    Chunk *getChunk(int i) const;
    friend class PacketDescriptor;
};

inline std::ostream& operator<<(std::ostream& os, const Packet *packet) { return os << packet->str(); }

inline std::ostream& operator<<(std::ostream& os, const Packet& packet) { return os << packet.str(); }

} // namespace

#endif // #ifndef __INET_PACKET_H_


