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

#ifndef EIGRPNETWORKTABLE_H_
#define EIGRPNETWORKTABLE_H_

#include "inet/routing/eigrp/EigrpDualStack.h"
namespace inet {
namespace eigrp {
/**
 * Network for EIGRP routing.
 */
template<typename IPAddress>
class EigrpNetwork
{
  protected:
    int networkId;
    IPAddress address;
    IPAddress mask;

  public:
    EigrpNetwork(IPAddress &address, IPAddress &mask, int id) :
            networkId(id), address(address), mask(mask) {}

    const IPAddress& getAddress() const {
        return address;
    }

    void setAddress(const IPAddress& address) {
        this->address = address;
    }

    const IPAddress& getMask() const {
        return mask;
    }

    void setMask(const IPAddress& mask) {
        this->mask = mask;
    }

    int getNetworkId() const {
        return networkId;
    }

    void setNetworkId(int networkId) {
        this->networkId = networkId;
    }
};

/**
 * Table with networks for routing.
 */
template<typename IPAddress>
class EigrpNetworkTable : cObject
{
  protected:
    std::vector<EigrpNetwork<IPAddress> *> networkVec;
    int networkCnt;

  public:
    static const int UNSPEC_NETID = 0;

    EigrpNetworkTable() : networkCnt(1) { }
    virtual ~EigrpNetworkTable();

    EigrpNetwork<IPAddress> *addNetwork(IPAddress& address, IPAddress& mask);
    EigrpNetwork<IPAddress> *findNetworkById(int netId);
    typename std::vector<EigrpNetwork<IPAddress> *> *getAllNetworks() { return &networkVec; }
    bool isAddressIncluded(IPAddress& address, IPAddress& mask);
    /**
     * Returns true if interface with specified address is contained in EIGRP.
     * @param resultNetId ID of network that belongs to the interface. If the interface does not
     * belong to any network, it has undefined value.
     */
    bool isInterfaceIncluded(const IPAddress& ifAddress, const IPAddress& ifMask, int *resultNetId);
};
} //eigrp
} //inet
#endif /* EIGRPNETWORKTABLE_H_ */
