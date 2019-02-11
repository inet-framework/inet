//
// Copyright (C) 2016 OpenSim Ltd.
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

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"

namespace inet {
namespace ieee80211 {

void ModeSetListener::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        getContainingNicModule(this)->subscribe(modesetChangedSignal, this);
}

void ModeSetListener::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    Enter_Method_Silent("receiveSignal");
    if (signalID == modesetChangedSignal)
        modeSet = check_and_cast<physicallayer::Ieee80211ModeSet*>(obj);
}

} /* namespace ieee80211 */
} /* namespace inet */
