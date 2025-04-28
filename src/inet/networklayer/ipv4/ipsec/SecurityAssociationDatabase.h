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

#ifndef __INET_SECURITYASSOCIATIONDATABASE_H_
#define __INET_SECURITYASSOCIATIONDATABASE_H_

#include <vector>
#include "inet/common/INETDefs.h"
#include "inet/common/SimpleModule.h"
#include "inet/networklayer/ipv4/ipsec/IPsecRule.h"
#include "inet/networklayer/ipv4/ipsec/SecurityAssociation.h"

namespace inet {
namespace ipsec {

/**
 * Represents the IPsec Security Association Database (SAD).
 * The database is filled by the IPsec module.
 */
class INET_API SecurityAssociationDatabase : public SimpleModule
{
  private:
    std::vector<SecurityAssociation *> entries; // note: change to std::unordered_map<int,std::vector<>> with SPI as key if performance warrants that

  protected:
    virtual void initialize() override;
    void refreshDisplay() const override;

  public:
    ~SecurityAssociationDatabase();

    /**
     * Add an security association (SA) to the database.
     * Other mutation operations are missing because the
     * database is built statically.
     */
    void addEntry(SecurityAssociation *sadEntry);

    /**
     * Look up an SA by SPI and direction. Useful for looking up the SA for an
     * ingress IPsec-protected packet. May later be extended with he possibility
     * of using addresses and DSCP as additional search keys.
     */
    SecurityAssociation *findEntry(IPsecRule::Direction direction, unsigned int spi);
};

}    // namespace ipsec
}    // namespace inet

#endif // ifndef __INET_SECURITYASSOCIATIONDATABASE_H_

