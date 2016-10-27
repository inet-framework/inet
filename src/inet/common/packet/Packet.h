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

class Packet : public cPacket
{
  protected:
    std::shared_ptr<SequenceChunk> data;
    SequenceChunk::ForwardIterator headerIterator;
    SequenceChunk::BackwardIterator trailerIterator;

  public:
    explicit Packet(const char *name = nullptr, short kind = 0);
    Packet(const Packet& other);

    virtual Packet *dup() const override { return new Packet(*this); }

    bool isImmutable() const { return data->isImmutable(); }
    bool isMutable() const { return !data->isImmutable(); }
    void assertMutable() const { data->assertMutable(); }
    void assertImmutable() const { data->assertImmutable(); }
    void makeImmutable() { data->makeImmutable(); }

    int64_t getHeaderPosition() const { return headerIterator.getPosition(); }
    void setHeaderPosition(int64_t offset) { headerIterator.seek(offset); }

    int64_t getTrailerPosition() const { return trailerIterator.getPosition(); }
    void setTrailerPosition(int64_t offset) { trailerIterator.seek(offset); }

    template <typename T>
    bool hasHeader(int64_t byteLength = -1) const {
        return data->peek<T>(headerIterator, byteLength) != nullptr;
    }
    template <typename T>
    std::shared_ptr<T> peekHeader(int64_t byteLength = -1) const {
        return data->peek<T>(headerIterator, byteLength);
    }
    std::shared_ptr<SliceChunk> peekHeader(int64_t byteLength = -1) const {
        return data->peek(headerIterator, byteLength);
    }
    template <typename T>
    std::shared_ptr<T> popHeader(int64_t byteLength = -1) {
        const auto& chunk = data->peek<T>(headerIterator, byteLength);
        if (chunk != nullptr)
            headerIterator.move(chunk->getByteLength());
        return chunk;
    }

    template <typename T>
    std::shared_ptr<T> peekHeaderAt(int64_t byteOffset, int64_t byteLength = -1) const {
        return data->peekAt<T>(headerIterator, byteOffset, byteLength);
    }
    std::shared_ptr<SliceChunk> peekHeaderAt(int64_t byteOffset, int64_t byteLength) const {
        return data->peekAt(headerIterator, byteOffset, byteLength);
    }

    template <typename T>
    bool hasTrailer(int64_t byteLength = -1) const {
        return data->peek<T>(headerIterator, byteLength) != nullptr;
    }
    template <typename T>
    std::shared_ptr<T> peekTrailer(int64_t byteLength = -1) const {
        return data->peek<T>(trailerIterator, byteLength);
    }
    std::shared_ptr<SliceChunk> peekTrailer(int64_t byteLength = -1) const {
        return data->peek(trailerIterator, byteLength);
    }
    template <typename T>
    std::shared_ptr<T> popTrailer(int64_t byteLength = -1) {
        auto& chunk = data->peek<T>(trailerIterator, byteLength);
        if (chunk != nullptr)
            trailerIterator.move(-chunk->getByteLength());
        return chunk;
    }

    template <typename T>
    std::shared_ptr<T> peekTrailerAt(int64_t byteOffset, int64_t byteLength = -1) const {
        return data->peekAt<T>(trailerIterator, byteOffset, byteLength);
    }
    std::shared_ptr<SliceChunk> peekTrailerAt(int64_t byteOffset, int64_t byteLength) const {
        return data->peekAt(trailerIterator, byteOffset, byteLength);
    }

    void prepend(const std::shared_ptr<Chunk>& chunk, bool flatten = true) { data->prepend(chunk, flatten); }
    void prepend(Packet *packet, bool flatten = true) { data->prepend(packet->data, flatten); }

    void append(const std::shared_ptr<Chunk>& chunk, bool flatten = true) { data->append(chunk, flatten); }
    void append(Packet *packet, bool flatten = true) { data->append(packet->data, flatten); }

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


