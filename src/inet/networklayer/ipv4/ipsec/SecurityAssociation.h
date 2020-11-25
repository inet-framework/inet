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
//

#ifndef __INET_SECURITYASSOCIATION_H_
#define __INET_SECURITYASSOCIATION_H_

#include "inet/common/INETDefs.h"
#include "inet/networklayer/ipv4/ipsec/IPsecRule.h"

namespace inet {
namespace ipsec {

/**
 * Represents an IPsec Security Association (SA).
 *
 * Note that several fields listed in RFC 4301 section 4.4.2.1
 * do not occur here due to simplifications in the model.
 * For example, the choice of AH/ESP cryptographic algorithm,
 * their parameters, keys, etc. are omitted because the model
 * does not perform actual cryptography. SA lifetime is omitted
 * because all SAs are statically configured, etc.
 */
class INET_API SecurityAssociation
{
  private:
    IPsecRule rule;
    unsigned int spi = 0;
    unsigned int seqNum = 0;

  public:
    typedef IPsecRule::Action Action;
    typedef IPsecRule::Direction Direction;
    typedef IPsecRule::Protection Protection;
    typedef IPsecRule::EspMode EspMode;
    typedef IPsecRule::EncryptionAlg EncryptionAlg;
    typedef IPsecRule::AuthenticationAlg AuthenticationAlg;

  public:
    const IPsecRule& getRule() const { return rule; }
    void setRule(const IPsecRule& rule) { this->rule = rule; }
    unsigned int getSpi() const { return spi; }
    void setSpi(unsigned int spi) { this->spi = spi; }
    Direction getDirection() const { return rule.getDirection(); }
    void setDirection(Direction direction) { rule.setDirection(direction); }
    Action getAction() const { return rule.getAction(); }
    void setAction(Action action) { rule.setAction(action); }
    Protection getProtection() const { return rule.getProtection(); }
    void setProtection(Protection protection) { rule.setProtection(protection); }
    EspMode getEspMode() const { return rule.getEspMode(); }
    void setEspMode(EspMode espMode) { rule.setEspMode(espMode); }
    EncryptionAlg getEnryptionAlg() const { return rule.getEnryptionAlg(); }
    void setEnryptionAlg(EncryptionAlg alg) { rule.setEnryptionAlg(alg); }
    AuthenticationAlg getAuthenticationAlg() const {return rule.getAuthenticationAlg(); }
    void setAuthenticationAlg(AuthenticationAlg alg) { rule.setAuthenticationAlg(alg); }
    unsigned int getMaxTfcPadLength() const { return rule.getMaxTfcPadLength(); }
    void setMaxTfcPadLength(unsigned int maxTfcPadLength) { rule.setMaxTfcPadLength(maxTfcPadLength); }
    unsigned int getSeqNum() const { return seqNum; }
    unsigned int getAndIncSeqNum() { return seqNum++; }
    void setSeqNum(unsigned int seqNum = 0) { this->seqNum = seqNum; }
    std::string str() const;
};

inline std::ostream& operator<<(std::ostream& os, const SecurityAssociation& e)
{
    return os << e.str();
}

}    // namespace ipsec
}    // namespace inet

#endif // ifndef __INET_SECURITYASSOCIATION_H_

