//
// Copyright (C) OpenSim Ltd
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

#ifndef __INET_SHORTCUTMAC_H
#define __INET_SHORTCUTMAC_H

#include "inet/common/INETMath.h"
#include "inet/linklayer/base/MacProtocolBase.h"

namespace inet {

class INET_API ShortcutMac : public MacProtocolBase
{
  protected:
    static std::map<MacAddress, ShortcutMac *> shortcutMacs;

  protected:
    double bitrate = NaN;
    cPar *propagationDelay = nullptr;
    cPar *lengthOverhead = nullptr;
    cPar *durationOverhead = nullptr;
    cPar *packetLoss = nullptr;

  public:
    ~ShortcutMac();

  protected:
    virtual void initialize(int stage) override;
    virtual void configureInterfaceEntry() override;

    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual void handleUpperPacket(Packet *packet) override;
    virtual void handleLowerPacket(Packet *packet) override;

    virtual ShortcutMac *findPeer(MacAddress address);
    virtual void sendToPeer(Packet *packet, ShortcutMac *peer);
    virtual void receiveFromPeer(Packet *packet);

    void deleteCurrentTxFrame() override { throw cRuntimeError("model error"); }
    void dropCurrentTxFrame(PacketDropDetails& details) override { throw cRuntimeError("model error"); }
    void popTxQueue() override { throw cRuntimeError("model error"); }
};

} // namespace inet

#endif // ifndef __INET_SHORTCUTMAC_H

