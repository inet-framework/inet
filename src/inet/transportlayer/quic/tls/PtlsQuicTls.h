//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PTLSQUICTLS_H
#define __INET_PTLSQUICTLS_H

#include "IQuicTls.h"
#include "IQuicTlsCallbacks.h"
#include "../packet/QuicPacket.h"

extern "C" {
#include <picotls.h>
#include <picotls/openssl_opp.h>
}

namespace inet {
namespace quic {

/**
 * PicoTLS-based implementation of the IQuicTls interface.
 *
 * This class provides QUIC packet protection and header protection
 * using the picotls library with AES-128-GCM cipher suite.
 */
class PtlsQuicTls : public IQuicTls
{
  protected:
    EncryptionKey key;
    IQuicTlsCallbacks *callbacks = nullptr;

    /**
     * Determines the packet number offset by parsing the packet header.
     * Handles both long and short header formats.
     */
    static size_t determinePnOffset(const uint8_t *data, size_t len);

    /**
     * Gets the packet number length from the first byte (encoded in low 2 bits).
     */
    static size_t getPnLength(uint8_t firstByte);

    /**
     * Checks if this is a long header packet.
     */
    static bool isLongHeader(uint8_t firstByte);

  public:
    /**
     * Constructs a PtlsQuicTls instance with the given encryption key.
     *
     * @param key The encryption key containing key, iv, and hpkey.
     * @param callbacks Optional callbacks for TLS events (may be nullptr).
     */
    PtlsQuicTls(const EncryptionKey& key, IQuicTlsCallbacks *callbacks = nullptr);

    virtual ~PtlsQuicTls();

    /**
     * Sets the encryption key to use for protection operations.
     */
    void setKey(const EncryptionKey& key) { this->key = key; }

    /**
     * Gets the current encryption key.
     */
    const EncryptionKey& getKey() const { return key; }

    /**
     * Sets the callbacks interface.
     */
    void setCallbacks(IQuicTlsCallbacks *callbacks) { this->callbacks = callbacks; }

    // IQuicTls interface implementation
    virtual bool protectPacket(std::vector<uint8_t>& packet, size_t pnOffset, uint64_t pn) override;
    virtual void protectHeader(std::vector<uint8_t>& packet, size_t pnOffset) override;
    virtual void unprotectHeader(std::vector<uint8_t>& packet, size_t pnOffset) override;
    virtual bool unprotectPacket(std::vector<uint8_t>& packet, size_t pnOffset, uint64_t pn) override;

    /**
     * Static helper to determine packet number offset from raw packet data.
     * This can be used before creating a PtlsQuicTls instance.
     */
    static size_t determinePnOffset(const std::vector<uint8_t>& packet);
};

/**
 * Factory function to create a PtlsQuicTls instance.
 */
inline IQuicTls* createPtlsQuicTls(const EncryptionKey& key, IQuicTlsCallbacks *callbacks = nullptr)
{
    return new PtlsQuicTls(key, callbacks);
}

} // namespace quic
} // namespace inet

#endif // __INET_PTLSQUICTLS_H
