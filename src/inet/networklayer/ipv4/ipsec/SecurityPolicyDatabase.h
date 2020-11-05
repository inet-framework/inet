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

#ifndef __INET_SECURITYPOLICYDATABASE_H_
#define __INET_SECURITYPOLICYDATABASE_H_

#include "inet/common/INETDefs.h"
#include "inet/networklayer/ipv4/ipsec/IPsecRule.h"
#include "inet/networklayer/ipv4/ipsec/SecurityAssociationDatabase.h"
#include "inet/networklayer/ipv4/ipsec/SecurityPolicy.h"

namespace inet {
namespace ipsec {

/**
 * Represents the IPsec Policy Association Database (SPD).
 * The database is filled by the IPsec module.
 */
class INET_API SecurityPolicyDatabase : public cSimpleModule
{
  private:
    std::vector<SecurityPolicy *> entries;

  protected:
    virtual void initialize() override;
    void refreshDisplay() const override;

  public:
    ~SecurityPolicyDatabase();

    /**
     * Add an security association (SA) to the database.
     * Other mutation operations are missing because the
     * database is built statically.
     */
    void addEntry(SecurityPolicy *entry);

    /**
     * Find the first matching security policy for a packet (given with its
     * selector fields) and direction. Useful for finding the applicable policy
     * for an egress packet.
     */
    SecurityPolicy *findEntry(IPsecRule::Direction direction, PacketInfo *packet);
};

}  // namespace ipsec
}  // namespace inet

#endif // ifndef __INET_SECURITYPOLICYDATABASE_H_

