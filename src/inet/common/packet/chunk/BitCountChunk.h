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

#ifndef __INET_BITCOUNTCHUNK_H_
#define __INET_BITCOUNTCHUNK_H_

#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

/**
 * This class represents data using a bit length field only. This can be useful
 * when the actual data is irrelevant and memory efficiency is high priority.
 */
class INET_API BitCountChunk : public Chunk
{
  friend class Chunk;

  protected:
    /**
     * The chunk length in bits, or -1 if not yet specified.
     */
    bit length;

  protected:
    virtual std::shared_ptr<Chunk> peekUnchecked(std::function<bool(const std::shared_ptr<Chunk>&)> predicate, std::function<const std::shared_ptr<Chunk>(const std::shared_ptr<Chunk>& chunk, const Iterator& iterator, bit length)> converter, const Iterator& iterator, bit length, int flags) const override;

    static std::shared_ptr<Chunk> createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, bit offset, bit length);

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    BitCountChunk();
    BitCountChunk(const BitCountChunk& other);
    BitCountChunk(bit length);

    virtual BitCountChunk *dup() const override { return new BitCountChunk(*this); }
    virtual std::shared_ptr<Chunk> dupShared() const override { return std::make_shared<BitCountChunk>(*this); }
    //@}

    /** @name Field accessor functions */
    //@{
    bit getLength() const { return length; }
    void setLength(bit length);
    //@}

    /** @name Overridden chunk functions */
    //@{
    virtual ChunkType getChunkType() const override { return CT_BITCOUNT; }
    virtual bit getChunkLength() const override { return length; }

    virtual bool canInsertAtBeginning(const std::shared_ptr<Chunk>& chunk) override;
    virtual bool canInsertAtEnd(const std::shared_ptr<Chunk>& chunk) override;

    virtual void insertAtBeginning(const std::shared_ptr<Chunk>& chunk) override;
    virtual void insertAtEnd(const std::shared_ptr<Chunk>& chunk) override;

    virtual bool canRemoveFromBeginning(bit length) override { return true; }
    virtual bool canRemoveFromEnd(bit length) override { return true; }

    virtual void removeFromBeginning(bit length) override;
    virtual void removeFromEnd(bit length) override;

    virtual std::string str() const override;
    //@}
};

} // namespace

#endif // #ifndef __INET_BITCOUNTCHUNK_H_

