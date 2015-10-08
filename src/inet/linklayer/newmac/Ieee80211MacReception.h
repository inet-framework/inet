//
// Copyright (C) 2015 Andras Varga
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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// Author: Andras Varga
//

#ifndef IEEE80211MACRECEPTION_H_
#define IEEE80211MACRECEPTION_H_

#include "Ieee80211MacPlugin.h"
#include "inet/common/INETDefs.h"
#include "inet_old/linklayer/contract/RadioState.h"

namespace inet {

class Ieee80211NewMac;
class Ieee80211Frame;
class Ieee80211UpperMac;

class Ieee80211MacReception : public Ieee80211MacPlugin
{
    public:
        cMessage *nav = nullptr;

        /** Physical radio (medium) state copied from physical layer */
        RadioState::State radioState;

        void handleMessage(cMessage *msg);

    public:
        void radioStateChanged(RadioState::State newRadioState);
        /** @brief Tells if the medium is free according to the physical and virtual carrier sense algorithm. */
        virtual bool isMediumFree() const;
        void setNav(simtime_t navInterval);
        void handleLowerFrame(Ieee80211Frame *frame);

        Ieee80211MacReception(Ieee80211NewMac *mac);

};

} /* namespace inet */

#endif /* IEEE80211MACRECEPTION_H_ */
