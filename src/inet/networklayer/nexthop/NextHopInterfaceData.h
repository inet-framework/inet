//
// Copyright (C) 2012 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_NEXTHOPINTERFACEDATA_H
#define __INET_NEXTHOPINTERFACEDATA_H

#include <vector>

#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

/**
 * Next hop forwarding protocol specific data for an NetworkInterface, stores generic network address.
 *
 * @see NetworkInterface
 */
class INET_API NextHopInterfaceData : public InterfaceProtocolData
{
  protected:
    L3Address inetAddr;    ///< address of interface
    int metric;    ///< link "cost"; see e.g. MS KB article Q299540  //TODO needed???

  public:
    // field ids for change notifications
    enum { F_ADDRESS, F_METRIC };

  protected:
    void changed1(int fieldId) { changed(interfaceGnpConfigChangedSignal, fieldId); }

  private:
    // copying not supported: following are private and also left undefined
    NextHopInterfaceData(const NextHopInterfaceData& obj);
    NextHopInterfaceData& operator=(const NextHopInterfaceData& obj);

  public:
    NextHopInterfaceData() : InterfaceProtocolData(NetworkInterface::F_NEXTHOP_DATA) { metric = 0; }
    virtual std::string str() const override;
    virtual std::string detailedInfo() const;

    /** @name Getters */
    //@{
    L3Address getAddress() const { return inetAddr; }
    int getMetric() const { return metric; }
    //@}

    /** @name Setters */
    //@{
    virtual void setAddress(L3Address a) { inetAddr = a; changed1(F_ADDRESS); }
    virtual void setMetric(int m) { metric = m; changed1(F_METRIC); }
    //@}

    virtual void joinMulticastGroup(L3Address address) {}
};

} // namespace inet

#endif

