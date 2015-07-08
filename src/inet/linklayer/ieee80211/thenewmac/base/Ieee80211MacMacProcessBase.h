//
// Copyright (C) 2015 OpenSim Ltd.
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

#ifndef __INET_IEEE80211MACMACPROCESSBASE_H
#define __INET_IEEE80211MACMACPROCESSBASE_H

#include "inet/common/INETDefs.h"
#include "inet/physicallayer/base/packetlevel/PhysicalLayerDefs.h"
#include "inet/linklayer/ieee80211/thenewmac/base/SdlProcess.h"
#include "inet/linklayer/ieee80211/thenewmac/signals/Ieee80211MacSignals_m.h"

namespace inet {
namespace ieee80211 {

class INET_API Ieee80211MacMacProcessBase : public cSimpleModule
{
    protected:
        cMessage *resetMac = nullptr;
        SdlProcess *sdlProcess = nullptr;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        void handleMessage(cMessage *msg) override;

        cMessage *createSignal(const char *name, Ieee80211MacSignal *signal);
        void createSignal(cPacket *packet, Ieee80211MacSignal *signal);

    public:
        virtual ~Ieee80211MacMacProcessBase();

        virtual void emitResetMac();
};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211MACMACPROCESSBASE_H
