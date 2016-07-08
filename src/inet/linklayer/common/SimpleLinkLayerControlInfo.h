//
// Copyright (C) 2011 Andras Varga
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

#ifndef __INET_SIMPLELINKLAYERCONTROLINFO_H
#define __INET_SIMPLELINKLAYERCONTROLINFO_H

#include "inet/common/Protocol.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/linklayer/common/SimpleLinkLayerControlInfo_m.h"

namespace inet {

/**
 * Represents a SimpleLinkLayer control info. More info in the SimpleLinkLayerControlInfo.msg file
 * (and the documentation generated from it).
 */
class INET_API SimpleLinkLayerControlInfo : public SimpleLinkLayerControlInfo_Base
{
  public:
    SimpleLinkLayerControlInfo() : SimpleLinkLayerControlInfo_Base() {}
    SimpleLinkLayerControlInfo(const SimpleLinkLayerControlInfo& other) : SimpleLinkLayerControlInfo_Base(other) {}
    SimpleLinkLayerControlInfo& operator=(const SimpleLinkLayerControlInfo& other) { SimpleLinkLayerControlInfo_Base::operator=(other); return *this; }

    virtual SimpleLinkLayerControlInfo *dup() const override { return new SimpleLinkLayerControlInfo(*this); }

    virtual int getNetworkProtocol() const { return SimpleLinkLayerControlInfo_Base::getProtocol(); }
    virtual void setNetworkProtocol(int protocol) { SimpleLinkLayerControlInfo_Base::setProtocol(protocol); }
    virtual int getSocketId() const override { return SimpleLinkLayerControlInfo_Base::getSocketId(); }
    virtual void setSocketId(int socketId) override { SimpleLinkLayerControlInfo_Base::setSocketId(socketId); }
};

} // namespace inet

#endif // ifndef __INET_SIMPLELINKLAYERCONTROLINFO_H

