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

#ifndef __INET_NETWORKLAYERNODEBASE_H_
#define __INET_NETWORKLAYERNODEBASE_H_

#include "inet/linklayer/base/LinkLayerNodeBase.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"

namespace inet {

class NetworkLayerNodeBase : public LinkLayerNodeBase
{
  friend class NetworkLayerNodeBaseDescriptor;

  protected:
    int getNumIPv4Routes() const { return check_and_cast<IIPv4RoutingTable *>(getModuleByPath(".ipv4.routingTable"))->getNumRoutes(); } // only for class descriptor
    const IPv4Route *getIPv4Route(int i) const { return check_and_cast<IIPv4RoutingTable *>(getModuleByPath(".ipv4.routingTable"))->getRoute(i); } // only for class descriptor
};

} // namespace inet

#endif // __INET_NETWORKLAYERNODEBASE_H_
