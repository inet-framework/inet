//
// Copyright (C) 2020 OpenSim Ltd and Marcel Marek
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
//  along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef INET_NETWORKLAYER_IPV4_IPSEC_IPSECRULE_H_
#define INET_NETWORKLAYER_IPV4_IPSEC_IPSECRULE_H_

#include "inet/common/INETDefs.h"
#include "PacketSelector.h"
#include "Enum.h"

// Some Windows header defines IN and OUT as macros, which clashes with our use of those words.
#ifdef _WIN32
#  ifdef IN
#    undef IN
#  endif
#  ifdef OUT
#    undef OUT
#  endif
#endif

namespace inet {
namespace ipsec {

/**
 * Encapsulates an IPsec rule: "if packet matches the given direction and selector,
 * take action DISCARD, BYPASS or PROTECT, and for the last one, protection
 * should be AH, ESP or both".
 */
class INET_API IPsecRule
{
  public:
    enum class Direction {
        INVALID,
        IN,
        OUT
    };

    enum class Action {
        NONE,
        DISCARD,
        BYPASS,
        PROTECT
    };

    enum class Protection {
        NONE,
        AH,
        ESP
    };

    enum class EspMode {
        NONE,
        INTEGRITY,
        CONFIDENTIALITY,
        COMBINED
    };

    enum class EncryptionAlg {
        /*
         * "NULL" algorithm, see RFC 4303 Section 3.2.1
         */
        NONE, // Note: the name NULL is still reserved due to the C heritage
        /*
         * DES RFC 2405
         * BS = 8B
         * IV = 8B
         */
        DES, //  MUST NOT RFC 8221 5.
        /*
         * 3DES RFC 2451
         * Key size = 24B
         * BS = 8B
         * IV = 8B
         *
         */
        TRIPLE_DES, // SHOULD NOT RFC 8221 5.
        /*
         * AES_CBC RFC 3602
         * BS = 16B
         * IV = same as Block Size
         */
        AES_CBC_128, AES_CBC_192, AES_CBC_256,
        /*
         * AES_GCM RFC 4106
         * GCM is a block cipher mode of operation providing both confidentiality and data origin authentication.
         * BS = 16B
         * IV = 8B
         * ICV = 16B (MUST) 8B, 12B - MAY
         * The Key Length attribute MUST have a value of 128, 192, or 256.
         */
        AES_GCM_16_128, AES_GCM_16_192, AES_GCM_16_256, //Combined mode
        /*
         * AES_CCM RFC 4309
         * BS = 16B
         * IV = 8B
         * ICV = 8B or 16B (MUST), 12B - MAY
         */
        AES_CCM_8_128, AES_CCM_8_192, AES_CCM_8_256,    //Combined mode
        AES_CCM_16_128, AES_CCM_16_192, AES_CCM_16_256,  //Combined mode
        /*
         * CHACHA20_POLY1305 RFC 7634
         * IV = 8B
         * ICV = 16B - MUST, 8B, 12B - MAY
         * !!!! It is a stream cipher, but when used in ESP the ciphertext needs to be aligned so that padLength and nextHeader are right aligned to multiple of 4 octets.
         * BS = 4B
         */
        CHACHA20_POLY1305 // Combined mode
    };

    enum class AuthenticationAlg {
        NONE,              // "NULL" algorithm, RFC 4301 Section 3.2.2
        HMAC_MD5_96,       // MUST NOT RFC 8221 Section 6
        HMAC_SHA1,         // MUST-    RFC 8221 Section 6
        AES_128_GMAC,      // MAY      RFC 8221 Section 6
        AES_192_GMAC,      // MAY      RFC 8221 Section 6
        AES_256_GMAC,      // MAY      RFC 8221 Section 6
        HMAC_SHA2_256_128, // MUST     RFC 8221 Section 6
        HMAC_SHA2_384_192,
        HMAC_SHA2_512_256, // SHOULD   RFC 8221 Section 6
    };

  private:
    Direction direction = Direction::INVALID;
    PacketSelector selector;
    Action action = Action::DISCARD;
    Protection protection = Protection::NONE;  // if action=PROTECT
    EspMode espMode = EspMode::NONE;
    EncryptionAlg enryptionAlg = EncryptionAlg::NONE;
    AuthenticationAlg authenticationAlg = AuthenticationAlg::NONE;
    unsigned int maxTfcPadLength;

  public:
    Direction getDirection() const { return direction; }
    void setDirection(Direction direction) { this->direction = direction; }
    const PacketSelector& getSelector() const { return selector; }
    void setSelector(const PacketSelector& selector) { this->selector = selector; }
    Action getAction() const { return action; }
    void setAction(Action action = Action::DISCARD) { this->action = action; }
    Protection getProtection() const { return protection; }
    void setProtection(Protection protection) { this->protection = protection; }
    EspMode getEspMode() const { return espMode; }
    void setEspMode(EspMode espMode) { this->espMode = espMode; }
    EncryptionAlg getEnryptionAlg() const { return enryptionAlg; }
    void setEnryptionAlg(EncryptionAlg alg) { this->enryptionAlg = alg; }
    AuthenticationAlg getAuthenticationAlg() const {return authenticationAlg; }
    void setAuthenticationAlg(AuthenticationAlg alg) { this->authenticationAlg = alg; }
    unsigned int getMaxTfcPadLength() const { return maxTfcPadLength; }
    void setMaxTfcPadLength(unsigned int maxTfcPadLength) { this->maxTfcPadLength = maxTfcPadLength; }
    std::string str() const;
};

inline std::ostream& operator<<(std::ostream& os, const IPsecRule& e)
{
    return os << e.str();
}

extern Enum<IPsecRule::Direction> directionEnum;
extern Enum<IPsecRule::Action> actionEnum;
extern Enum<IPsecRule::Protection> protectionEnum;
extern Enum<IPsecRule::EspMode> espModeEnum;
extern Enum<IPsecRule::EncryptionAlg> encryptionAlgEnum;
extern Enum<IPsecRule::AuthenticationAlg> authenticationAlgEnum;

}  // namespace ipsec
}  // namespace inet

#endif /* INET_NETWORKLAYER_IPV4_IPSEC_IPSECRULE_H_ */

