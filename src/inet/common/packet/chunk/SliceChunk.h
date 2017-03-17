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

#ifndef __INET_SLICECHUNK_H_
#define __INET_SLICECHUNK_H_

#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

/**
 * This class represents data using a slice of another chunk. The slice is
 * designated with the sliced chunk, an offset, and a length field, both
 * measured in bytes. It's used by the Chunk API implementation internally to
 * efficiently represent slices of other chunks. User code should not directly
 * instantiate this class.
 */
class INET_API SliceChunk : public Chunk
{
  friend class Chunk;
  friend class SliceChunkDescriptor;

  protected:
    /**
     * The chunk of which this chunk is a slice, or nullptr if not yet specified.
     */
    std::shared_ptr<Chunk> chunk;
    /**
     * The offset measured in bytes, or -1 if not yet specified.
     */
    bit offset;
    /**
     * The length measured in bytes, or -1 if not yet specified.
     */
    bit length;

  protected:
    Chunk *_getChunk() const { return chunk.get(); } // only for class descriptor

    static std::shared_ptr<Chunk> createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, bit offset, bit length);

    virtual std::shared_ptr<Chunk> peekSliceChunk(const Iterator& iterator, bit length = bit(-1)) const override;

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    SliceChunk();
    SliceChunk(const SliceChunk& other);
    SliceChunk(const std::shared_ptr<Chunk>& chunk, bit offset, bit length);

    virtual SliceChunk *dup() const override { return new SliceChunk(*this); }
    virtual std::shared_ptr<Chunk> dupShared() const override { return std::make_shared<SliceChunk>(*this); }
    //@}

    /** @name Field accessor functions */
    //@{
    const std::shared_ptr<Chunk>& getChunk() const { return chunk; }
    void setChunk(const std::shared_ptr<Chunk>& chunk) { this->chunk = chunk; }

    bit getOffset() const { return offset; }
    void setOffset(bit offset);

    bit getLength() const { return length; }
    void setLength(bit length);
    //@}

    /** @name Overridden chunk functions */
    //@{
    virtual Type getChunkType() const override { return TYPE_SLICE; }
    virtual bit getChunkLength() const override { return length; }

    virtual bool canInsertAtBeginning(const std::shared_ptr<Chunk>& chunk) override;
    virtual bool canInsertAtEnd(const std::shared_ptr<Chunk>& chunk) override;

    virtual void insertAtBeginning(const std::shared_ptr<Chunk>& chunk) override;
    virtual void insertAtEnd(const std::shared_ptr<Chunk>& chunk) override;

    virtual bool canRemoveFromBeginning(bit length) override { return false; }
    virtual bool canRemoveFromEnd(bit length) override { return false; }

    virtual void removeFromBeginning(bit length) override;
    virtual void removeFromEnd(bit length) override;

    virtual std::shared_ptr<Chunk> peek(const Iterator& iterator, bit length = bit(-1), int flags = 0) const override;

    virtual std::string str() const override;
    //@}
};

} // namespace

#endif // #ifndef __INET_SLICECHUNK_H_

