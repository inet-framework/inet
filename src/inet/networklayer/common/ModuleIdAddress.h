//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MODULEIDADDRESS_H
#define __INET_MODULEIDADDRESS_H

#include <iostream>
#include <string>

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * This class provides network addresses using the module id of interface modules.
 * The module id address supports unspecified, broadcast and multicast addresses too.
 * This class doesn't support address prefixes.
 */
class INET_API ModuleIdAddress
{
  private:
    int id;

  public:
    ModuleIdAddress() : id(0) {}
    ModuleIdAddress(int id) : id(id) {}

    int getId() const { return id; }
    bool tryParse(const char *addr);

    bool isUnspecified() const { return id == 0; }
    bool isUnicast() const { return id > 0; }
    bool isMulticast() const { return id < -1; }
    bool isBroadcast() const { return id == -1; }

    /**
     * Returns equals(addr).
     */
    bool operator==(const ModuleIdAddress& addr1) const { return id == addr1.id; }

    /**
     * Returns !equals(addr).
     */
    bool operator!=(const ModuleIdAddress& addr1) const { return id != addr1.id; }

    /**
     * Compares two addresses.
     */
    bool operator<(const ModuleIdAddress& addr1) const { return id < addr1.id; }
    bool operator<=(const ModuleIdAddress& addr1) const { return id <= addr1.id; }
    bool operator>(const ModuleIdAddress& addr1) const { return id > addr1.id; }
    bool operator>=(const ModuleIdAddress& addr1) const { return id >= addr1.id; }

    std::string str() const { std::ostringstream s; s << id; return s.str(); }
};

} // namespace inet

#endif

