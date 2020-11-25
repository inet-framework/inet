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

#include "inet/networklayer/ipv4/ipsec/IPsecRule.h"

namespace inet {
namespace ipsec {

Enum<IPsecRule::Direction> directionEnum {
#define P(NAME)  {IPsecRule::Direction::NAME,#NAME}
    P(INVALID),
    P(IN),
    P(OUT)
#undef P
};

Enum<IPsecRule::Action> actionEnum {
#define P(NAME)  {IPsecRule::Action::NAME,#NAME}
    P(NONE),
    P(DISCARD),
    P(BYPASS),
    P(PROTECT)
#undef P
};

Enum<IPsecRule::Protection> protectionEnum {
#define P(NAME)  {IPsecRule::Protection::NAME,#NAME}
    P(NONE),
    P(AH),
    P(ESP)
#undef P
};

Enum<IPsecRule::EspMode> espModeEnum {
#define P(NAME)  {IPsecRule::EspMode::NAME,#NAME}
    P(NONE),
    P(INTEGRITY),
    P(CONFIDENTIALITY),
    P(COMBINED)
#undef P
};

Enum<IPsecRule::EncryptionAlg> encryptionAlgEnum {
#define P(NAME)  {IPsecRule::EncryptionAlg::NAME,#NAME}
    P(NONE),
    P(AES_CBC_128), P(AES_CBC_192), P(AES_CBC_256),
    P(DES),
    P(TRIPLE_DES),
    P(AES_CCM_8_128), P(AES_CCM_8_192), P(AES_CCM_8_256),
    P(AES_CCM_16_128), P(AES_CCM_16_192), P(AES_CCM_16_256),
    P(AES_GCM_16_128), P(AES_GCM_16_192), P(AES_GCM_16_256)

#undef P
};

Enum<IPsecRule::AuthenticationAlg> authenticationAlgEnum {
#define P(NAME)  {IPsecRule::AuthenticationAlg::NAME,#NAME}
    P(NONE),
    P(HMAC_MD5_96),
    P(HMAC_SHA1),
    P(AES_128_GMAC),
    P(AES_192_GMAC),
    P(AES_256_GMAC),
    P(HMAC_SHA2_256_128),
    P(HMAC_SHA2_384_192),
    P(HMAC_SHA2_512_256)
#undef P
};

std::string IPsecRule::str() const
{
    std::stringstream out;
    out << "Rule: " << selector;
    out << " Direction: " << directionEnum.nameOf(direction);
    out << " Action: " << actionEnum.nameOf(action);
    if (action == Action::PROTECT) {
        out << " with: " << protectionEnum.nameOf(protection);
        if (protection == Protection::AH)
            out << " / " << authenticationAlgEnum.nameOf(authenticationAlg);
        else if (protection == Protection::ESP) {
            out << " mode " << espModeEnum.nameOf(espMode);
            if (espMode == EspMode::CONFIDENTIALITY || espMode == EspMode::COMBINED) {
                out << " encryptAlg:" << encryptionAlgEnum.nameOf(enryptionAlg);
                out << " maxTfcPadLen:" << maxTfcPadLength;
            }
            if (espMode == EspMode::INTEGRITY || espMode == EspMode::COMBINED)
                out << " authAlg:" << authenticationAlgEnum.nameOf(authenticationAlg);
        }
    }
    return out.str();
}

}    // namespace ipsec
}    // namespace inet

