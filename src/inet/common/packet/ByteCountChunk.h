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

#include "inet/common/packet/Chunk.h"

namespace inet {

/**
 * This class represents data using a length field only. This can be useful
 * when the actual data is irrelevant and memory efficiency is high priority.
 */
class INET_API ByteCountChunk : public Chunk
{
  friend Chunk;

  protected:
    /**
     * The chunk length in bytes, or -1 if not yet specified.
     */
    int64_t length;

  protected:
    static std::shared_ptr<Chunk> createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length);

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    ByteCountChunk();
    ByteCountChunk(const ByteCountChunk& other);
    ByteCountChunk(int64_t length);

    virtual ByteCountChunk *dup() const override { return new ByteCountChunk(*this); }
    virtual std::shared_ptr<Chunk> dupShared() const override { return std::make_shared<ByteCountChunk>(*this); }
    //@}

    /** @name Field accessor functions */
    //@{
    int64_t getLength() const { return length; }
    void setLength(int64_t length);
    //@}

    /** @name Overridden chunk functions */
    //@{
    virtual Type getChunkType() const override { return TYPE_LENGTH; }
    virtual int64_t getChunkLength() const override { return length; }

    virtual bool insertAtBeginning(const std::shared_ptr<Chunk>& chunk) override;
    virtual bool insertAtEnd(const std::shared_ptr<Chunk>& chunk) override;

    virtual bool removeFromBeginning(int64_t length) override;
    virtual bool removeFromEnd(int64_t length) override;

    virtual std::shared_ptr<Chunk> peek(const Iterator& iterator, int64_t length = -1) const override;

    virtual std::string str() const override;
    //@}
};

} // namespace

#endif // #ifndef __INET_BYTECOUNTCHUNK_H_

