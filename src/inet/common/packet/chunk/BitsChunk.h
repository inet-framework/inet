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

#ifndef __INET_BITSCHUNK_H_
#define __INET_BITSCHUNK_H_

#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

/**
 * This class represents data using a sequence of bits. This can be useful
 * when the actual data is important because. For example, when an external
 * program sends or receives the data, or in hardware in the loop simulations.
 */
class INET_API BitsChunk : public Chunk
{
  friend class Chunk;

  protected:
    /**
     * The data bits as is.
     */
    std::vector<bool> bits;

  protected:
    virtual std::shared_ptr<Chunk> peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, bit length, int flags) const override;

    static std::shared_ptr<Chunk> convertChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, bit offset, bit length);

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    BitsChunk();
    BitsChunk(const BitsChunk& other);
    BitsChunk(const std::vector<bool>& bits);

    virtual BitsChunk *dup() const override { return new BitsChunk(*this); }
    virtual std::shared_ptr<Chunk> dupShared() const override { return std::make_shared<BitsChunk>(*this); }
    //@}

    /** @name Field accessor functions */
    //@{
    const std::vector<bool>& getBits() const { return bits; }
    void setBits(const std::vector<bool>& bits);

    bool getBit(int index) const { return bits[index]; }
    void setBit(int index, bool bit);
    //@}

    /** @name Overridden chunk functions */
    //@{
    virtual ChunkType getChunkType() const override { return CT_BITS; }
    virtual bit getChunkLength() const override { return bit(bits.size()); }

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

#endif // #ifndef __INET_BITSCHUNK_H_

