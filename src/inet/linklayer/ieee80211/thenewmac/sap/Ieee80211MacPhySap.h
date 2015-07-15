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

#ifndef __INET_IEEE80211MACPHYSAP_H
#define __INET_IEEE80211MACPHYSAP_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/ieee80211/thenewmac/signals/Ieee80211MacSignals_m.h"
#include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacEnumeratedPhyTypes.h"
#include "inet/linklayer/ieee80211/thenewmac/base/Ieee80211MacMacProcessBase.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

class INET_API IIeee80211MacPhySap
{
    protected:
        virtual void emitPhyCcaIndication() = 0;
};


class INET_API Ieee80211MacPhySap : public Ieee80211MacMacProcessBase
{
    protected:
        IRadio *radio = nullptr;

        // Upper layers
        cGate *phySapTxInput = nullptr;
        cGate *phySapTxOutput = nullptr;
        cGate *phySapRxInput = nullptr;
        cGate *phySapRxOutput = nullptr;

        // Lower layers
        cGate *radioIn = nullptr;
        cGate *radioOut = nullptr;

    protected:
        virtual void handleMessage(cMessage *msg) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        void initialize(int stage);

        void configureRadioMode(IRadio::RadioMode radioMode);

    protected:
        virtual void emitPhyCcaIndication();
};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211MACPHYSAP_H
