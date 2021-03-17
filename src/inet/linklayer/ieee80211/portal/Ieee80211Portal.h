//
// Copyright (C) 2020 OpenSim Ltd.
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

#ifndef __INET_IEEE80211PORTAL_H
#define __INET_IEEE80211PORTAL_H

#include "inet/common/Protocol.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/FcsMode_m.h"
#include "inet/linklayer/ieee80211/llc/IIeee80211Llc.h"
#include "inet/linklayer/ieee8022/Ieee8022Llc.h"

namespace inet {
namespace ieee80211 {

class INET_API Ieee80211Portal : public Ieee8022Llc, public IIeee80211Llc
{
  protected:
    FcsMode fcsMode = FCS_MODE_UNDEFINED;
    bool upperLayerOutConnected = false;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void processPacketFromMac(Packet *packet) override;

  public:
    const Protocol *getProtocol() const override { return &Protocol::ieee8022llc; }
};

} // namespace ieee80211
} // namespace inet

#endif

