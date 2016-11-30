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

#include "Chunk.h"

namespace inet {

/**
 * This class represents data using a sequence of bytes. This can be useful
 * when the actual data is important because. For example, when an external
 * program sends or receives the data, or in hardware in the loop simulations.
 */
class INET_API BytesChunk : public Chunk
{
  friend Chunk;

  protected:
    /**
     * The data bytes as is.
     */
    std::vector<uint8_t> bytes;

  protected:
    static std::shared_ptr<Chunk> createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length);

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    BytesChunk();
    BytesChunk(const BytesChunk& other);
    BytesChunk(const std::vector<uint8_t>& bytes);

    virtual BytesChunk *dup() const override { return new BytesChunk(*this); }
    virtual std::shared_ptr<Chunk> dupShared() const override { return std::make_shared<BytesChunk>(*this); }
    //@}

    /** @name Field accessor functions */
    //@{
    const std::vector<uint8_t>& getBytes() const { return bytes; }
    void setBytes(const std::vector<uint8_t>& bytes);

    uint8_t getByte(int index) const { return bytes[index]; }
    void setByte(int index, uint8_t byte);
    //@}

    /** @name Overridden chunk functions */
    //@{
    virtual Type getChunkType() const override { return TYPE_BYTES; }
    virtual int64_t getChunkLength() const override { return bytes.size(); }

    virtual bool insertAtBeginning(const std::shared_ptr<Chunk>& chunk) override;
    virtual bool insertAtEnd(const std::shared_ptr<Chunk>& chunk) override;

    virtual bool removeFromBeginning(int64_t length) override;
    virtual bool removeFromEnd(int64_t length) override;

    virtual std::shared_ptr<Chunk> peek(const Iterator& iterator, int64_t length = -1) const override;

    virtual std::string str() const override;
    //@}
};

} // namespace

#endif // #ifndef __INET_BYTESCHUNK_H_

