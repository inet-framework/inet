// Copyright (C) 2012 - 2016 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
 * @file CLNSAddress.h
 * @author Marcel Marek (mailto:imarek@fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 10.8.2016
 * @brief Class representing a CLNS Address
 * @detail Class representing a CLNS Address. It should be probably called NSAPAddress or something similar.
 */

#ifndef INET_NETWORKLAYER_CLNS_CLNSADDRESS_H_
#define INET_NETWORKLAYER_CLNS_CLNSADDRESS_H_

#include <iostream>
#include <string>

#include "inet/common/INETDefs.h"

namespace inet {

class ClnsAddress
{
  private:
    uint64 systemID;
    uint64 areaID;
    uint8 nsel; //this field is probably not part of NET, but part of NSAP (still confused)

  public:
    enum AddressCategory {
        UNSPECIFIED    // 00.0000.0000.0000.0000.00
    };

    static const ClnsAddress UNSPECIFIED_ADDRESS;

    ClnsAddress();
    ClnsAddress(std::string net);
    ClnsAddress(uint64 areaID, uint64 systemID, uint8 nsel = 0);   //FIXME remove nsel initialization
    void set(uint64 areaID, uint64 systemID, uint8 nsel = 0);   //FIXME remove nsel initialization
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
    bool equals(const ClnsAddress& toCmp) const { return (systemID == toCmp.systemID && areaID == toCmp.areaID); }

    uint64 getAreaId() const;
    uint64 getSystemId() const;
    uint8 getNsel() const;
    void setNsel(uint8 nsel);

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
    bool operator<(const ClnsAddress& addr1) const { if(areaID == addr1.getAreaId()){return systemID < addr1.getSystemId();}else{ return areaID < addr1.getAreaId();} }
    bool operator<=(const ClnsAddress& addr1) const { if(areaID == addr1.getAreaId()){return systemID <= addr1.getSystemId();}else{ return areaID < addr1.getAreaId();} }
    bool operator>(const ClnsAddress& addr1) const { if(areaID == addr1.getAreaId()){return systemID > addr1.getSystemId();}else{ return areaID > addr1.getAreaId();} }
    bool operator>=(const ClnsAddress& addr1) const { if(areaID == addr1.getAreaId()){return systemID >= addr1.getSystemId();}else{ return areaID > addr1.getAreaId();} }
};

inline std::ostream& operator<<(std::ostream& os, const ClnsAddress& net)
{
    return os << net.str();
}

} //end of namespace inet

#endif /* INET_NETWORKLAYER_CLNS_CLNSADDRESS_H_ */

