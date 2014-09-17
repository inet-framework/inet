//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2005 Wei Yang, Ng
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

//  Cleanup and rewrite: Andras Varga, 2004
//  Implementation of IPv6 version: Wei Yang, Ng, 2005

#ifndef __INET_IPV6ERRORHANDLING_H
#define __INET_IPV6ERRORHANDLING_H

#include "inet/common/INETDefs.h"

#include "inet/networklayer/contract/ipv6/IPv6ControlInfo.h"
#include "inet/networklayer/icmpv6/ICMPv6Message_m.h"

namespace inet {

/**
 * Error Handling: print out received error for IPv6
 */
// FIXME is such thing needed at all???
class INET_API IPv6ErrorHandling : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

  protected:
    virtual void displayType1Msg(int code);
    virtual void displayType2Msg();
    virtual void displayType3Msg(int code);
    virtual void displayType4Msg(int code);
};

} // namespace inet

#endif // ifndef __INET_IPV6ERRORHANDLING_H

