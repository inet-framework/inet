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

#ifndef __INET_EIGRPNETWORKTABLE_H
#define __INET_EIGRPNETWORKTABLE_H

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
    EigrpNetwork(IPAddress& address, IPAddress& mask, int id) :
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
class INET_API EigrpNetworkTable : cObject
{
  protected:
    std::vector<EigrpNetwork<IPAddress> *> networkVec;
    int networkCnt;

  public:
    static const int UNSPEC_NETID = 0;

    EigrpNetworkTable() : networkCnt(1) {}
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
} // eigrp
} // inet
#endif

