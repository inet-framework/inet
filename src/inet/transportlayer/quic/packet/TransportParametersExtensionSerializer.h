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

#ifndef __INET_TRANSPORTPARAMETERSEXTENSIONSERIALIZER_H
#define __INET_TRANSPORTPARAMETERSEXTENSIONSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/transportlayer/quic/packet/FrameHeader_m.h"
#include "inet/transportlayer/quic/packet/VariableLengthInteger_m.h"

namespace inet {
namespace quic {

/**
 * Converts between QUIC Transport Parameters Extension and binary TLS extension representation.
 */
class INET_API TransportParametersExtensionSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

    // Helper methods for variable length integers
    virtual void serializeVariableLengthInteger(MemoryOutputStream& stream, uint64_t value) const;
    virtual uint64_t deserializeVariableLengthInteger(MemoryInputStream& stream) const;

  public:
    TransportParametersExtensionSerializer() : FieldsChunkSerializer() {}
};

} // namespace quic
} // namespace inet

#endif // __INET_TRANSPORTPARAMETERSEXTENSIONSERIALIZER_H
