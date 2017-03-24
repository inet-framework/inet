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

  protected:
    virtual std::shared_ptr<Chunk> peekUnchecked(std::function<bool(const std::shared_ptr<Chunk>&)> predicate, std::function<const std::shared_ptr<Chunk>(const std::shared_ptr<Chunk>& chunk, const Iterator& iterator, bit length)> converter, const Iterator& iterator, bit length, int flags) const override;

    static std::shared_ptr<Chunk> createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, bit offset, bit length);

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    ByteCountChunk();
    ByteCountChunk(const ByteCountChunk& other);
    ByteCountChunk(byte length);

    virtual ByteCountChunk *dup() const override { return new ByteCountChunk(*this); }
    virtual std::shared_ptr<Chunk> dupShared() const override { return std::make_shared<ByteCountChunk>(*this); }
    //@}

    /** @name Field accessor functions */
    //@{
    byte getLength() const { return length; }
    void setLength(byte length);
    //@}

    /** @name Overridden chunk functions */
    //@{
    virtual ChunkType getChunkType() const override { return CT_BYTECOUNT; }
    virtual bit getChunkLength() const override { return length; }

    virtual bool canInsertAtBeginning(const std::shared_ptr<Chunk>& chunk) override;
    virtual bool canInsertAtEnd(const std::shared_ptr<Chunk>& chunk) override;

    virtual void insertAtBeginning(const std::shared_ptr<Chunk>& chunk) override;
    virtual void insertAtEnd(const std::shared_ptr<Chunk>& chunk) override;

    virtual bool canRemoveFromBeginning(bit length) override { return bit(length).get() % 8 == 0; }
    virtual bool canRemoveFromEnd(bit length) override { return bit(length).get() % 8 == 0; }

    virtual void removeFromBeginning(bit length) override;
    virtual void removeFromEnd(bit length) override;

    virtual std::string str() const override;
    //@}
};

} // namespace

#endif // #ifndef __INET_BYTECOUNTCHUNK_H_

