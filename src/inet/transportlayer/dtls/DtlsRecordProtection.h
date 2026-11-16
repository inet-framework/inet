// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_DTLSRECORDPROTECTION_H
#define INET_DTLSRECORDPROTECTION_H

#include "inet/common/packet/Packet.h"

namespace inet::dtls {

/** The modeled AES-GCM record payload: inner content type and 16-byte tag. */
class INET_API DtlsRecordProtection
{
  public:
    static constexpr int TAG_BYTES = 16;
    static void protect(Packet *packet, uint8_t innerContentType);
    // On success replaces the protected payload with its retained content.
    // A false result leaves the packet unchanged. No cryptographic check occurs.
    static bool unprotect(Packet *packet, uint8_t& innerContentType);
};

} // namespace inet::dtls
#endif
