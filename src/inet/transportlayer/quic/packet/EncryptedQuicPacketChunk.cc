#include "../../../transportlayer/quic/packet/EncryptedQuicPacketChunk.h"

namespace inet {
namespace quic {

Register_Class(EncryptedQuicPacketChunk);

EncryptedQuicPacketChunk::EncryptedQuicPacketChunk() :
    EncryptedChunk()
{
}

EncryptedQuicPacketChunk::EncryptedQuicPacketChunk(const Ptr<Chunk>& chunk, b length) :
    EncryptedChunk(chunk, length)
{
}

EncryptedQuicPacketChunk *EncryptedQuicPacketChunk::dup() const
{
    return new EncryptedQuicPacketChunk(*this);
}

const Ptr<Chunk> EncryptedQuicPacketChunk::dupShared() const
{
    return makeShared<EncryptedQuicPacketChunk>(*this);
}

} // namespace quic
} // namespace inet
