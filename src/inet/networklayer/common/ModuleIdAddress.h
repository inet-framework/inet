//
// Copyright (C) 2012 Opensim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
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

#endif // ifndef __INET_MODULEIDADDRESS_H

