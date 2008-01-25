//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

//
//  Author: Andras Varga
//

#ifndef __IPv4INTERFACEDATA_H
#define __IPv4INTERFACEDATA_H

#include <vector>
#include <omnetpp.h>
#include "INETDefs.h"
#include "IPAddress.h"


/**
 * IPv4-specific data in an InterfaceEntry. Stores interface IP address,
 * netmask, metric, etc.
 *
 * @see InterfaceEntry
 */
class INET_API IPv4InterfaceData : public cPolymorphic
{
  public:
    typedef std::vector<IPAddress> IPAddressVector;

  private:
    IPAddress _inetAddr;    ///< IP address of interface
    IPAddress _netmask;     ///< netmask
    int _metric;            ///< link "cost"; see e.g. MS KB article Q299540
    IPAddressVector _multicastGroups; ///< multicast groups

  private:
    // copying not supported: following are private and also left undefined
    IPv4InterfaceData(const IPv4InterfaceData& obj);
    IPv4InterfaceData& operator=(const IPv4InterfaceData& obj);

  public:
    IPv4InterfaceData();
    virtual ~IPv4InterfaceData() {}
    virtual std::string info() const;
    virtual std::string detailedInfo() const;

    IPAddress inetAddress() const  {return _inetAddr;}
    IPAddress netmask() const      {return _netmask;}
    int metric() const             {return _metric;}
    const IPAddressVector& multicastGroups() const {return _multicastGroups;}

    void setInetAddress(IPAddress a) {_inetAddr = a;}
    void setNetmask(IPAddress m)     {_netmask = m;}
    void setMetric(int m)            {_metric = m;}
    void setMulticastGroups(const IPAddressVector& v) {_multicastGroups = v;}
};

#endif

