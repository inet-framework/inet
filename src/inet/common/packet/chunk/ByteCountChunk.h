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

#ifndef __INET_BYTECOUNTCHUNK_H_
#define __INET_BYTECOUNTCHUNK_H_

#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

/**
 * This class represents data using a byte length field only. This can be useful
 * when the actual data is irrelevant and memory efficiency is high priority.
 */
class INET_API ByteCountChunk : public Chunk
{
  friend class Chunk;

  protected:
    /**
     * The chunk length in bytes, or -1 if not yet specified.
     */
    byte length;
    uint8_t data;

  protected:
    virtual const Ptr<Chunk> peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, bit length, int flags) const override;

    static const Ptr<Chunk> convertChunk(const std::type_info& typeInfo, const Ptr<Chunk>& chunk, bit offset, bit length, int flags);

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    ByteCountChunk();
    ByteCountChunk(const ByteCountChunk& other);
    ByteCountChunk(byte length, uint8_t data = '?');

    virtual ByteCountChunk *dup() const override { return new ByteCountChunk(*this); }
    virtual const Ptr<Chunk> dupShared() const override { return std::make_shared<ByteCountChunk>(*this); }
    //@}

    /** @name Field accessor functions */
    //@{
    byte getLength() const { return length; }
    void setLength(byte length);

    uint8_t getData() const { return data; }
    void setData(uint8_t data);
    //@}

    /** @name Overridden chunk functions */
    //@{
    virtual ChunkType getChunkType() const override { return CT_BYTECOUNT; }
    virtual bit getChunkLength() const override { CHUNK_CHECK_IMPLEMENTATION(length >= byte(0)); return length; }

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

#endif // #ifndef __INET_BYTECOUNTCHUNK_H_

