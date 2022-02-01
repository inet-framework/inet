//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MODULEPATHADDRESS_H
#define __INET_MODULEPATHADDRESS_H

#include <iostream>
#include <string>

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * This class provides network addresses using the module path to interface modules.
 * The module path address supports unspecified, broadcast and multicast addresses too.
 * TODO add support for partial module paths addresses to allow prefix routing
 */
class INET_API ModulePathAddress
{
  private:
    int id;

  public:
    ModulePathAddress() : id(0) {}
    ModulePathAddress(int id) : id(id) {}

    int getId() const { return id; }
    bool tryParse(const char *addr);

    bool isUnspecified() const { return id == 0; }
    bool isUnicast() const { return id > 0; }
    bool isMulticast() const { return id < -1; }
    bool isBroadcast() const { return id == -1; }

    /**
     * Returns equals(addr).
     */
    bool operator==(const ModulePathAddress& addr1) const { return id == addr1.id; }

    /**
     * Returns !equals(addr).
     */
    bool operator!=(const ModulePathAddress& addr1) const { return id != addr1.id; }

    /**
     * Compares two addresses.
     */
    bool operator<(const ModulePathAddress& addr1) const { return id < addr1.id; }
    bool operator<=(const ModulePathAddress& addr1) const { return id <= addr1.id; }
    bool operator>(const ModulePathAddress& addr1) const { return id > addr1.id; }
    bool operator>=(const ModulePathAddress& addr1) const { return id >= addr1.id; }
    static bool maskedAddrAreEqual(const ModulePathAddress& addr1, const ModulePathAddress& addr2, int prefixLength) { return addr1.id == addr2.id; }

    std::string str() const;
};

} // namespace inet

#endif

