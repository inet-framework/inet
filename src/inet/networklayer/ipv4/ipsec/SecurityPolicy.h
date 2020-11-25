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

#ifndef __INET_SECURITYPOLICY_H_
#define __INET_SECURITYPOLICY_H_

#include "inet/common/INETDefs.h"
#include "inet/networklayer/ipv4/ipsec/IPsecRule.h"
#include "inet/networklayer/ipv4/ipsec/SecurityAssociation.h"

namespace inet {
namespace ipsec {

/**
 * Represents an IPsec Security Policy (a.k.a. SPD entry), together
 * with SAs created for this policy.
 *
 * Note that several fields listed in RFC 4301 section 4.4.1.2 do not occur
 * here due to simplifications in the model. For example, PFP flags are not
 * needed because SAs are configured statically.
 */
class INET_API SecurityPolicy
{
  private:
    IPsecRule rule;
    std::vector<SecurityAssociation*> entries; // entries owned by SAD

  public:
    typedef IPsecRule::Action Action;
    typedef IPsecRule::Direction Direction;
    typedef IPsecRule::Protection Protection;
    typedef IPsecRule::EspMode EspMode;
    typedef IPsecRule::EncryptionAlg EncryptionAlg;
    typedef IPsecRule::AuthenticationAlg AuthenticationAlg;

  public:
    Action getAction() const { return rule.getAction(); }
    void setAction(Action action) { rule.setAction(action); }
    Direction getDirection() const { return rule.getDirection(); }
    void setDirection(Direction direction) { rule.setDirection(direction); }
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
    const PacketSelector& getSelector() const { return rule.getSelector(); }
    void setSelector(const PacketSelector& selector) { rule.setSelector(selector); }
    const IPsecRule& getRule() const { return rule; }
    void setRule(const IPsecRule& rule) { this->rule = rule; }
    const std::vector<SecurityAssociation *>& getEntries() const { return entries; }
    void setEntries(const std::vector<SecurityAssociation *>& sadEntryList) { this->entries = sadEntryList; }
    void addEntry(SecurityAssociation *sadEntry) { entries.push_back(sadEntry); }
    std::string str() const;
};

}  // namespace ipsec
}  // namespace inet

#endif // ifndef __INET_SECURITYPOLICY_H_

