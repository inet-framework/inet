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

#ifndef __INET_ORIGNETWORKDATAGRAMTAG_H
#define __INET_ORIGNETWORKDATAGRAMTAG_H

#include "inet/common/Protocol.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/networklayer/common/OrigNetworkDatagramTag_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"

namespace inet {

/**
 * Original Network Datagram indication
 */
class INET_API OrigNetworkDatagramInd : public OrigNetworkDatagramInd_Base
{
  protected:
    cPacket *dgram = nullptr;

  private:
    void copy(const OrigNetworkDatagramInd& other);
    void clean();

  public:
    OrigNetworkDatagramInd() : OrigNetworkDatagramInd_Base() { dgram = nullptr; }
    virtual ~OrigNetworkDatagramInd();
    OrigNetworkDatagramInd(const OrigNetworkDatagramInd& other) : OrigNetworkDatagramInd_Base(other) { dgram = nullptr; copy(other); }
    OrigNetworkDatagramInd& operator=(const OrigNetworkDatagramInd& other);
    virtual OrigNetworkDatagramInd *dup() const override { return new OrigNetworkDatagramInd(*this); }

    virtual void setOrigDatagram(cPacket *d);
    virtual cPacket *getOrigDatagram() const { return dgram; }
    virtual cPacket *removeOrigDatagram();

};

} // namespace inet

#endif // ifndef __INET_ORIGNETWORKDATAGRAMTAG_H

