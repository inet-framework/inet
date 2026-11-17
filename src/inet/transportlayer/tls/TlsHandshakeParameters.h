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
    static constexpr uint16_t AES_256_GCM_SHA384 = 0x1302;
    static constexpr uint16_t CHACHA20_POLY1305_SHA256 = 0x1303;
    static constexpr uint16_t X25519 = 0x001d;
    static constexpr uint16_t SECP256R1 = 0x0017;
    static constexpr uint16_t ECDSA_P256_SHA256 = 0x0403;
    static constexpr uint16_t RSA_PSS_RSAE_SHA256 = 0x0804;
    static constexpr uint8_t PSK_DHE_KE = 1;

    enum class Profile { PSK, CERTIFICATE };
    enum class AuthenticationMode { CERTIFICATE = 0, EXTERNAL_PSK = 1, TICKET_PSK = 2 };
    struct Preferences {
        std::vector<uint16_t> cipherSuites = {AES_128_GCM_SHA256};
        std::vector<uint16_t> groups = {X25519};
        std::vector<uint16_t> signatures = {ECDSA_P256_SHA256};
    };
    struct CipherSuite {
        uint16_t code;
        const char *name;
        const char *parameterPrefix;
        uint8_t hashBytes;
        uint8_t tagBytes;
        uint64_t recordLimit;
        uint64_t rekeyLimit;
    };
    struct Offer {
        bool hasEarlyData = false;
        bool hasPostHandshakeAuth = false;
        bool hasConnectionId = false;
        uint8_t heartbeatMode = 0;
        std::vector<uint8_t> connectionId;
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
        std::vector<uint8_t> binderLengths;
    };
    struct Selection {
        uint16_t version = DTLS_13;
        uint16_t cipherSuite = AES_128_GCM_SHA256;
        uint16_t group = X25519;
        uint16_t signature = 0;
        uint16_t selectedPsk = 0;
        AuthenticationMode authenticationMode = AuthenticationMode::CERTIFICATE;
        [[nodiscard]] bool usesPsk() const { return authenticationMode != AuthenticationMode::CERTIFICATE; }
        bool retryKeyShare = false;
    };

    // Returns an explanation on incompatibility; an empty string is success.
    static std::string select(const Offer& offer, Profile profile, const std::string& identity, Selection& selection);
    static std::string select(const Offer& offer, Profile profile, const std::string& identity, Selection& selection,
                              const Preferences& preferences, uint8_t pskHashBytes);
    static const CipherSuite *findCipherSuite(uint16_t code);
    static size_t keyShareBytes(uint16_t group);
    static Preferences parsePreferences(const char *cipherSuites, const char *groups, const char *signatures);
    static bool wasOffered(const Offer& offer, const Selection& selection);
    // Decode the standard ClientHello extension vectors without retaining
    // pointers into the supplied body. Empty return means successful decoding.
    static std::string decodeOffer(const Ptr<const TlsClientHello>& hello, Offer& offer);
};

} // namespace inet::tls
#endif
