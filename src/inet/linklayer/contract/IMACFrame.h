//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_IMACFRAME_H
#define __INET_IMACFRAME_H

#include "inet/linklayer/common/MACAddress.h"

namespace inet {

/**
 * This purely virtual interface provides an abstraction for different link layer frames.
 */
class INET_API IMACFrame
{
  public:
    virtual ~IMACFrame() {}
    virtual const MACAddress& getTransmitterAddress() const = 0;
    virtual void setTransmitterAddress(const MACAddress& address) = 0;
    virtual const MACAddress& getReceiverAddress() const = 0;
    virtual void setReceiverAddress(const MACAddress& address) = 0;
};

} // namespace inet

#endif // ifndef __INET_IMACFRAME_H

