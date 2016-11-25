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

#ifndef __INET_FIELDSCHUNK_H_
#define __INET_FIELDSCHUNK_H_

#include "inet/common/packet/Chunk.h"

namespace inet {

/**
 * TODO
 */
class FieldsChunk : public Chunk
{
  friend Chunk;

  protected:
    /**
    * The serialized representation of this chunk or nullptr if not available.
    * When a chunk is serialized, the result is stored here for fast subsequent
    * serializations. Moreover, if a chunk is created by deserialization, then
    * the original bytes are also stored here. The serialized representation
    * is deleted if a chunk is modified.
    */
    const std::vector<uint8_t> *serializedBytes;

  protected:
    virtual void handleChange() override;

  protected:
    static std::shared_ptr<Chunk> createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length) {
        return Chunk::createChunk(typeInfo, chunk, offset, length);
    }

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    FieldsChunk();
    FieldsChunk(const FieldsChunk& other);
    virtual ~FieldsChunk();
    //@}

    /** @name Field accessor functions */
    //@{
    const std::vector<uint8_t> *getSerializedBytes() const { return serializedBytes; }
    void setSerializedBytes(const std::vector<uint8_t> *bytes) { this->serializedBytes = bytes; }
    //@}

    /** @name Overridden chunk functions */
    //@{
    virtual Type getChunkType() const override { return TYPE_FIELDS; }
    //@}
};

} // namespace

#endif // #ifndef __INET_FIELDCHUNK_H_

