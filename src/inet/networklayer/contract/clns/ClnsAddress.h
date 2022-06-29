//
// Copyright (C) 2012 - 2016 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


/**
 * @file CLNSAddress.h
 * @author Marcel Marek (mailto:imarek@fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 10.8.2016
 * @brief Class representing a CLNS Address
 * @detail Class representing a CLNS Address. It should be probably called NSAPAddress or something similar.
 */

#ifndef __INET_CLNSADDRESS_H
#define __INET_CLNSADDRESS_H

#include <iostream>
#include <string>

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API ClnsAddress
{
  private:
    uint64_t systemID;
    uint64_t areaID;
    uint8_t nsel; // this field is probably not part of NET, but part of NSAP (still confused)

  public:
    enum AddressCategory {
        UNSPECIFIED // 00.0000.0000.0000.0000.00
    };

    static const ClnsAddress UNSPECIFIED_ADDRESS;

    ClnsAddress();
    ClnsAddress(std::string net);
    ClnsAddress(uint64_t areaID, uint64_t systemID, uint8_t nsel = 0); // FIXME remove nsel initialization
    void set(uint64_t areaID, uint64_t systemID, uint8_t nsel = 0); // FIXME remove nsel initialization
    virtual ~ClnsAddress();

    bool isUnspecified() const;

    /**
     * Returns the string representation of the address (e.g. "49.0001.1921.6800.1001.00")
     * @param printUnspec: show 00.0000.0000.0000.0000.00 as "<unspec>" if true
     */
    std::string str(bool printUnspec = true) const;

    /**
     * Returns true if the two addresses are equal
     */
    bool equals(const ClnsAddress& toCmp) const { return systemID == toCmp.systemID && areaID == toCmp.areaID; }

    uint64_t getAreaId() const;
    uint64_t getSystemId() const;
    uint8_t getNsel() const;
    void setNsel(uint8_t nsel);

    /**
     * Returns equals(addr).
     */
    bool operator==(const ClnsAddress& addr1) const { return equals(addr1); }

    /**
     * Returns !equals(addr).
     */
    bool operator!=(const ClnsAddress& addr1) const { return !equals(addr1); }

    /**
     * Compares two CLNS addresses.
     */
    bool operator<(const ClnsAddress& addr1) const { if (areaID == addr1.getAreaId()) { return systemID < addr1.getSystemId(); } else { return areaID < addr1.getAreaId(); } }
    bool operator<=(const ClnsAddress& addr1) const { if (areaID == addr1.getAreaId()) { return systemID <= addr1.getSystemId(); } else { return areaID < addr1.getAreaId(); } }
    bool operator>(const ClnsAddress& addr1) const { if (areaID == addr1.getAreaId()) { return systemID > addr1.getSystemId(); } else { return areaID > addr1.getAreaId(); } }
    bool operator>=(const ClnsAddress& addr1) const { if (areaID == addr1.getAreaId()) { return systemID >= addr1.getSystemId(); } else { return areaID > addr1.getAreaId(); } }
};

inline std::ostream& operator<<(std::ostream& os, const ClnsAddress& net)
{
    return os << net.str();
}

} // namespace inet

#endif

