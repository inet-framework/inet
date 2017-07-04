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

#ifndef __INET_BYTESCHUNK_H_
#define __INET_BYTESCHUNK_H_

#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

/**
 * This class represents data using a sequence of bytes. This can be useful
 * when the actual data is important because. For example, when an external
 * program sends or receives the data, or in hardware in the loop simulations.
 */
class INET_API BytesChunk : public Chunk
{
  friend class Chunk;

  protected:
    /**
     * The data bytes as is.
     */
    std::vector<uint8_t> bytes;

  protected:
    virtual const Ptr<Chunk> peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, bit length, int flags) const override;

    static const Ptr<Chunk> convertChunk(const std::type_info& typeInfo, const Ptr<Chunk>& chunk, bit offset, bit length, int flags);

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    BytesChunk();
    BytesChunk(const BytesChunk& other);
    BytesChunk(const std::vector<uint8_t>& bytes);
    BytesChunk(const uint8_t *buffer, size_t bufLen) : Chunk(), bytes(buffer, buffer + bufLen) { }

    virtual BytesChunk *dup() const override { return new BytesChunk(*this); }
    virtual const Ptr<Chunk> dupShared() const override { return std::make_shared<BytesChunk>(*this); }
    //@}

    /** @name Field accessor functions */
    //@{
    const std::vector<uint8_t>& getBytes() const { return bytes; }
    void setBytes(const std::vector<uint8_t>& bytes);

    uint8_t getByte(int index) const { return bytes[index]; }
    void setByte(int index, uint8_t byte);
    //@}

    /** @name Utility functions */
    //@{
    size_t copyToBuffer(uint8_t *buffer, size_t bufferLength) const;
    void copyFromBuffer(const uint8_t *buffer, size_t bufferLength);
    //@}

    /** @name Overridden chunk functions */
    //@{
    virtual ChunkType getChunkType() const override { return CT_BYTES; }
    virtual bit getChunkLength() const override { return byte(bytes.size()); }

    virtual bool canInsertAtBeginning(const Ptr<const Chunk>& chunk) const override;
    virtual bool canInsertAtEnd(const Ptr<const Chunk>& chunk) const override;

    virtual void insertAtBeginning(const Ptr<const Chunk>& chunk) override;
    virtual void insertAtEnd(const Ptr<const Chunk>& chunk) override;

    virtual bool canRemoveFromBeginning(bit length) const override { return bit(length).get() % 8 == 0; }
    virtual bool canRemoveFromEnd(bit length) const override { return bit(length).get() % 8 == 0; }

    virtual void removeFromBeginning(bit length) override;
    virtual void removeFromEnd(bit length) override;

    virtual std::string str() const override;
    //@}
};

} // namespace

#endif // #ifndef __INET_BYTESCHUNK_H_

