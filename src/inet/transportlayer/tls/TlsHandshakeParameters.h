// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TLSHANDSHAKEPARAMETERS_H
#define INET_TLSHANDSHAKEPARAMETERS_H

#include <cstdint>
#include <string>
#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/transportlayer/tls/TlsClientHello_m.h"

namespace inet::tls {

/** Values supported by the selected modeled TLS 1.3 handshake profile. */
class INET_API TlsHandshakeParameters
{
  public:
    static constexpr uint16_t DTLS_13 = 0xfefc;
    static constexpr uint16_t AES_128_GCM_SHA256 = 0x1301;
    static constexpr uint16_t X25519 = 0x001d;
    static constexpr uint16_t ECDSA_P256_SHA256 = 0x0403;
    static constexpr uint8_t PSK_DHE_KE = 1;

    enum class Profile { PSK, CERTIFICATE };
    struct Offer {
        std::vector<uint16_t> versions;
        std::vector<uint16_t> cipherSuites;
        std::vector<uint16_t> groups;
        std::vector<uint16_t> keyShares;
        std::vector<uint16_t> signatures;
        std::vector<uint8_t> pskModes;
        bool hasSupportedGroups = false;
        bool hasKeyShare = false;
        bool hasSignatureAlgorithms = false;
        bool hasPskModes = false;
        bool hasPsk = false;
        std::string pskIdentity;
        std::vector<std::string> pskIdentities;
    };
    struct Selection {
        uint16_t version = DTLS_13;
        uint16_t cipherSuite = AES_128_GCM_SHA256;
        uint16_t group = X25519;
        uint16_t signature = 0;
        bool usePsk = false;
        bool retryKeyShare = false;
    };

    // Returns an explanation on incompatibility; an empty string is success.
    static std::string select(const Offer& offer, Profile profile, const std::string& identity, Selection& selection);
    static bool wasOffered(const Offer& offer, const Selection& selection);
    // Decode the standard ClientHello extension vectors without retaining
    // pointers into the supplied body. Empty return means successful decoding.
    static std::string decodeOffer(const Ptr<const TlsClientHello>& hello, Offer& offer);
};

} // namespace inet::tls
#endif
