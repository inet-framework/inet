// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_DTLS12RECORDPROTECTION_H
#define INET_DTLS12RECORDPROTECTION_H

#include "inet/common/packet/Packet.h"

namespace inet::dtls {

/** Selected DTLS 1.2 GCM record shape, retaining plaintext without cryptography. */
class INET_API Dtls12RecordProtection
{
  public:
    static constexpr int NONCE_BYTES = 8;
    static constexpr int TAG_BYTES = 16;
    static constexpr int MAX_CONTENT_BYTES = 1 << 14;
    static constexpr uint64_t RECORD_LIMIT = uint64_t(1) << 48;
    static uint64_t explicitNonce(uint16_t epoch, uint64_t sequence);
    static void protect(Packet *packet, uint16_t epoch, uint64_t sequence);
    // Failed unprotection leaves the packet unchanged. No cryptographic check occurs.
    static bool unprotect(Packet *packet, uint16_t epoch, uint64_t sequence);
};

} // namespace inet::dtls
#endif
