//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IPV6CONTROLINFO_H
#define __INET_IPV6CONTROLINFO_H

#include "inet/networklayer/contract/ipv6/IPv6ControlInfo_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"

namespace inet {

class IPv6Datagram;
class IPv6ExtensionHeader;

/**
 * Control information for sending/receiving packets over IPv6.
 *
 * See the IPv6ControlInfo.msg file for more info.
 */
class INET_API IPv6ControlInfo : public IPv6ControlInfo_Base
{
  private:
    void copy(const IPv6ControlInfo& other);
    void clean();

  public:
    IPv6ControlInfo() : IPv6ControlInfo_Base() { }
    virtual ~IPv6ControlInfo();
    IPv6ControlInfo(const IPv6ControlInfo& other) : IPv6ControlInfo_Base(other) { copy(other); }
    IPv6ControlInfo& operator=(const IPv6ControlInfo& other);
    virtual IPv6ControlInfo *dup() const override { return new IPv6ControlInfo(*this); }
};

} // namespace inet

#endif // ifndef __INET_IPV6CONTROLINFO_H

