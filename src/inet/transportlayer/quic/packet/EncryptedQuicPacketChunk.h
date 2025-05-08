#ifndef __INET_ENCRYPTEDQUICPACKETCHUNK_H
#define __INET_ENCRYPTEDQUICPACKETCHUNK_H

#include "inet/common/packet/chunk/EncryptedChunk.h"

namespace inet {
namespace quic {

/**
 * Represents an encrypted QUIC packet.
 * This is a specialized version of EncryptedChunk for QUIC packets.
 */
class INET_API EncryptedQuicPacketChunk : public EncryptedChunk
{
  protected:
    virtual EncryptedQuicPacketChunk *dup() const override;
    virtual const Ptr<Chunk> dupShared() const override;

  public:
    /**
     * @brief Default constructor.
     */
    EncryptedQuicPacketChunk();

    /**
     * @brief Copy constructor.
     */
    EncryptedQuicPacketChunk(const EncryptedQuicPacketChunk& other) = default;

    /**
     * @brief Constructor to create an encrypted chunk representing the given chunk.
     * @param chunk The chunk to be represented as encrypted.
     * @param length The length of the encrypted data.
     */
    EncryptedQuicPacketChunk(const Ptr<Chunk>& chunk, b length);
};

} // namespace quic
} // namespace inet

#endif // __INET_ENCRYPTEDQUICPACKETCHUNK_H
