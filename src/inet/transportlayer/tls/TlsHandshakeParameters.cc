// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/tls/TlsHandshakeParameters.h"

#include <algorithm>
#include <set>

namespace inet::tls {

template<typename T>
static bool contains(const std::vector<T>& values, T value)
{
    return std::find(values.begin(), values.end(), value) != values.end();
}

std::string TlsHandshakeParameters::select(const Offer& offer, Profile profile, const std::string& identity, Selection& selection)
{
    // RFC 9846 Sections 4.2.1 and 4.3: select only offered parameters; this model supports one profile per server.
    if (!contains(offer.versions, DTLS_13))
        return "no supported DTLS version";
    if (!contains(offer.cipherSuites, AES_128_GCM_SHA256))
        return "no supported cipher suite";
    if (!contains(offer.groups, X25519))
        return "no supported key exchange group";
    if (profile == Profile::PSK) {
        bool identityOffered = offer.pskIdentity == identity || contains(offer.pskIdentities, identity);
        if (!offer.hasPsk || !identityOffered || !contains(offer.pskModes, PSK_DHE_KE))
            return "no compatible external PSK";
    }
    else if (!contains(offer.signatures, ECDSA_P256_SHA256))
        return "no supported signature scheme";
    Selection result;
    result.authenticationMode = profile == Profile::PSK ? AuthenticationMode::EXTERNAL_PSK : AuthenticationMode::CERTIFICATE;
    result.signature = result.usesPsk() ? 0 : ECDSA_P256_SHA256;
    result.retryKeyShare = !contains(offer.keyShares, X25519);
    selection = result;
    return {};
}

std::string TlsHandshakeParameters::decodeOffer(const Ptr<const TlsClientHello>& hello, Offer& offer)
{
    // RFC 9846 Sections 4.2.2 and 4.3 define the ClientHello extension vectors.
    Offer result;
    for (size_t i = 0; i < hello->getCipherSuitesArraySize(); ++i)
        result.cipherSuites.push_back(hello->getCipherSuites(i));
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < hello->getExtensionsArraySize(); ++i)
        bytes.push_back(hello->getExtensions(i));
    auto read16 = [&](size_t& pos, size_t end, uint16_t& value) {
        if (end - pos < 2)
            return false;
        value = (uint16_t(bytes[pos]) << 8) | bytes[pos + 1];
        pos += 2;
        return true;
    };
    auto readList16 = [&](size_t& pos, size_t end, std::vector<uint16_t>& values) {
        uint16_t length;
        if (!read16(pos, end, length) || length == 0 || length % 2 || length != end - pos)
            return false;
        while (pos < end) {
            uint16_t value;
            if (!read16(pos, end, value))
                return false;
            values.push_back(value);
        }
        return true;
    };
    std::set<uint16_t> seen;
    size_t pos = 0;
    while (pos < bytes.size()) {
        uint16_t type, length;
        if (!read16(pos, bytes.size(), type) || !read16(pos, bytes.size(), length) || length > bytes.size() - pos)
            return "truncated extension";
        size_t end = pos + length;
        if (!seen.insert(type).second)
            return "duplicate extension";
        if (type == 43) { // supported_versions
            if (pos == end)
                return "invalid supported_versions";
            auto versionsLength = bytes[pos++];
            if (versionsLength != end - pos || (end - pos) == 0 || (end - pos) % 2)
                return "invalid supported_versions";
            while (pos < end) {
                uint16_t version;
                if (!read16(pos, end, version))
                    return "invalid supported_versions";
                result.versions.push_back(version);
            }
        }
        else if (type == 10 || type == 13) {
            if (type == 10)
                result.hasSupportedGroups = true;
            else
                result.hasSignatureAlgorithms = true;
            if (!readList16(pos, end, type == 10 ? result.groups : result.signatures))
                return "invalid algorithm list";
        }
        else if (type == 51) { // client_shares
            result.hasKeyShare = true;
            uint16_t listLength;
            if (!read16(pos, end, listLength) || listLength != end - pos)
                return "invalid key_share list";
            std::set<uint16_t> groups;
            while (pos < end) {
                uint16_t group, shareLength;
                if (!read16(pos, end, group) || !read16(pos, end, shareLength) || shareLength == 0 || shareLength > end - pos || !groups.insert(group).second)
                    return "invalid key_share";
                if (group == X25519 && shareLength != 32)
                    return "invalid X25519 key_share length";
                result.keyShares.push_back(group);
                pos += shareLength;
            }
        }
        else if (type == 45) {
            result.hasPskModes = true;
            if (pos == end)
                return "invalid PSK modes";
            auto modesLength = bytes[pos++];
            if (modesLength != end - pos || pos == end)
                return "invalid PSK modes";
            while (pos < end)
                result.pskModes.push_back(bytes[pos++]);
        }
        else if (type == 42) {
            if (length != 0)
                return "invalid early_data offer";
            result.hasEarlyData = true;
        }
        else if (type == 41) {
            if (end != bytes.size())
                return "pre_shared_key is not the last extension";
            uint16_t identityLength;
            if (!read16(pos, end, identityLength) || identityLength == 0 || identityLength > end - pos)
                return "invalid PSK identities";
            size_t identitiesEnd = pos + identityLength;
            while (pos < identitiesEnd) {
                uint16_t n;
                if (!read16(pos, identitiesEnd, n) || n == 0 || n > identitiesEnd - pos || identitiesEnd - pos - n < 4)
                    return "invalid PSK identity";
                result.pskIdentities.emplace_back(bytes.begin() + pos, bytes.begin() + pos + n);
                pos += n + 4; // identity and obfuscated ticket age
            }
            uint16_t binderLength;
            if (!read16(pos, end, binderLength) || binderLength != end - pos)
                return "invalid PSK binders";
            size_t binders = 0;
            while (pos < end) {
                auto n = bytes[pos++];
                if (n < 32 || n > end - pos)
                    return "invalid PSK binder";
                pos += n;
                ++binders;
            }
            if (binders != result.pskIdentities.size())
                return "PSK identity and binder counts differ";
            result.hasPsk = true;
            result.pskIdentity = result.pskIdentities.front();
        }
        pos = end; // unknown extensions remain opaque
    }
    if (result.hasEarlyData && !result.hasPsk)
        return "early_data requires a PSK offer";
    offer = std::move(result);
    return {};
}

bool TlsHandshakeParameters::wasOffered(const Offer& offer, const Selection& selection)
{
    if (selection.version != DTLS_13 || selection.cipherSuite != AES_128_GCM_SHA256 || selection.group != X25519 ||
        !contains(offer.versions, selection.version) || !contains(offer.cipherSuites, selection.cipherSuite) ||
        !contains(offer.groups, selection.group))
        return false;
    if (selection.retryKeyShare ? contains(offer.keyShares, selection.group) : !contains(offer.keyShares, selection.group))
        return false;
    if (selection.usesPsk())
        return offer.hasPsk && contains(offer.pskModes, PSK_DHE_KE) && selection.signature == 0;
    return selection.signature == ECDSA_P256_SHA256 && contains(offer.signatures, selection.signature);
}

} // namespace inet::tls
