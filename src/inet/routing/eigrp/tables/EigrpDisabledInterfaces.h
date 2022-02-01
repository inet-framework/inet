//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
/**
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

#ifndef __INET_EIGRPDISABLEDINTERFACES_H
#define __INET_EIGRPDISABLEDINTERFACES_H

#include "inet/routing/eigrp/tables/EigrpInterfaceTable.h"
namespace inet {
namespace eigrp {

/**
 * Table with disabled EIGRP interfaces. Used to store the settings of interfaces.
 */
class EigrpDisabledInterfaces
{
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

} // namespace eigrp
} // namespace inet
#endif

