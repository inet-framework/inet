//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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
/**
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

#ifndef EIGRPDISABLEDINTERFACES_H_
#define EIGRPDISABLEDINTERFACES_H_

#include "inet/routing/eigrp/tables/EigrpInterfaceTable.h"
namespace inet {
namespace eigrp {

/**
 * Table with disabled EIGRP interfaces. Used to store the settings of interfaces.
 */
class EigrpDisabledInterfaces {
  protected:
    std::vector<EigrpInterface *> ifVector;

  public:
    EigrpDisabledInterfaces();
    virtual ~EigrpDisabledInterfaces();

    /**
     * Returns number of interfaces in the table.
     */
    int getNumInterfaces() const { return ifVector.size(); }
    /**
     * Removes specified interface from table and returns it. If interface does not exist in the table, returns null.
     */
    EigrpInterface *removeInterface(EigrpInterface *iface);
    /**
     * Adds interface to the table.
     */
    void addInterface(EigrpInterface *interface);
    /**
     * Finds interface by ID in table and returns it. If interface with specified ID does not exist in the table, returns null.
     */
    EigrpInterface *findInterface(int ifaceId);
    /**
     * Returns interface by its position in the table.
     */
    EigrpInterface *getInterface(int k) const { return ifVector[k]; }
};

}// namespace eigrp
}// namespace inet
#endif /* EIGRPDISABLEDINTERFACES_H_ */
