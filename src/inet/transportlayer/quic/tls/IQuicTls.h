//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IQUICTLS_H
#define __INET_IQUICTLS_H

#include <vector>
#include <cstdint>

namespace inet {
namespace quic {

/**
 * Interface for QUIC packet protection and header protection operations.
 *
 * This interface abstracts the cryptographic operations needed for QUIC packet
 * encryption/decryption and header protection/unprotection. Implementations
 * can use different TLS libraries (e.g., picotls, OpenSSL, etc.).
 *
 * The protection operations follow RFC 9001 (QUIC-TLS):
 * - Packet protection uses AEAD (e.g., AES-128-GCM) to encrypt the payload
 *   and authenticate both header and payload
 * - Header protection masks certain header bytes to prevent ossification
 */
class IQuicTls
{
  public:
    virtual ~IQuicTls() = default;

    /**
     * Applies packet payload protection (AEAD encryption) and writes the
     * integrity tag at the end of the packet.
     *
     * @param packet The packet data (header + plaintext payload). Will be
     *               modified in place and extended to include the auth tag.
     * @param pnOffset The byte offset where the packet number begins.
     * @param pn The full packet number (used as AEAD nonce).
     * @return true on success, false on failure.
     */
    virtual bool protectPacket(std::vector<uint8_t>& packet, size_t pnOffset, uint64_t pn) = 0;

    /**
     * Applies header protection by XORing the first byte and packet number
     * bytes with a mask derived from a sample of the ciphertext.
     *
     * This should be called after protectPacket().
     *
     * @param packet The packet data (with protected payload).
     * @param pnOffset The byte offset where the packet number begins.
     */
    virtual void protectHeader(std::vector<uint8_t>& packet, size_t pnOffset) = 0;

    /**
     * Removes header protection to reveal the true first byte and packet number.
     *
     * This should be called before unprotectPacket().
     *
     * @param packet The packet data (with protected header).
     * @param pnOffset The byte offset where the packet number begins.
     */
    virtual void unprotectHeader(std::vector<uint8_t>& packet, size_t pnOffset) = 0;

    /**
     * Attempts to remove the payload protection (AEAD decryption) and verify
     * the integrity tag.
     *
     * @param packet The packet data (with unprotected header, protected payload).
     *               On success, will be modified to contain decrypted payload
     *               and resized to remove the auth tag.
     * @param pnOffset The byte offset where the packet number begins.
     * @param pn The expected full packet number (used as AEAD nonce).
     * @return true on successful decryption and authentication, false on failure.
     */
    virtual bool unprotectPacket(std::vector<uint8_t>& packet, size_t pnOffset, uint64_t pn) = 0;

    /**
     * Combined protect operation: applies both packet protection and header protection.
     * This is a convenience method that calls protectPacket() followed by protectHeader().
     *
     * @param packet The packet data to protect.
     * @param pnOffset The byte offset where the packet number begins.
     * @param pn The full packet number.
     * @return true on success, false on failure.
     */
    virtual bool protect(std::vector<uint8_t>& packet, size_t pnOffset, uint64_t pn)
    {
        if (!protectPacket(packet, pnOffset, pn))
            return false;
        protectHeader(packet, pnOffset);
        return true;
    }

    /**
     * Combined unprotect operation: removes header protection, extracts the packet number,
     * and removes payload protection.
     * This is a convenience method that calls unprotectHeader() followed by unprotectPacket().
     *
     * @param packet The protected packet data.
     * @param pnOffset The byte offset where the packet number begins.
     * @param expectedPn The expected full packet number.
     * @return true on success, false on failure.
     */
    virtual bool unprotect(std::vector<uint8_t>& packet, size_t pnOffset, uint64_t expectedPn)
    {
        unprotectHeader(packet, pnOffset);
        return unprotectPacket(packet, pnOffset, expectedPn);
    }
};

} // namespace quic
} // namespace inet

#endif // __INET_IQUICTLS_H
