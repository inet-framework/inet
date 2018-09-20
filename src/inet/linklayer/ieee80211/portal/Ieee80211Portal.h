//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_IEEE80211PORTAL_H
#define __INET_IEEE80211PORTAL_H

#include "inet/common/Protocol.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/FcsMode_m.h"
#include "inet/linklayer/ieee80211/llc/IIeee80211Llc.h"

namespace inet {

namespace ieee80211 {

class INET_API Ieee80211Portal : public cSimpleModule, public IIeee80211Llc
{
  protected:
    FcsMode fcsMode = FCS_MODE_UNDEFINED;
    bool upperLayerOutConnected = false;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void encapsulate(Packet *packet);
    virtual void decapsulate(Packet *packet);

  public:
    const Protocol *getProtocol() const override { return &Protocol::ieee8022; }
};

} // namespace ieee80211

} // namespace inet

#endif // __INET_IEEE80211PORTAL_H

