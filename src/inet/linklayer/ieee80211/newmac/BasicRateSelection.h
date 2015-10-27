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

#ifndef __INET_BASICRATESELECTION_H
#define __INET_BASICRATESELECTION_H

#include "IRateSelection.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {
namespace ieee80211 {

class INET_API BasicRateSelection : public cSimpleModule, public IRateSelection
{
    private:
        const Ieee80211ModeSet *modeSet;
        const IIeee80211Mode *dataFrameMode;  // only if rateControl == nullptr
        const IIeee80211Mode *multicastFrameMode;
        const IIeee80211Mode *controlFrameMode;
        const IIeee80211Mode *slowestMandatoryMode;
        IRateControl *rateControl = nullptr;  // optional

    public:
        BasicRateSelection();
        virtual void initialize() override;
        virtual void setRateControl(IRateControl *rateControl) override;
        virtual const IIeee80211Mode *getSlowestMandatoryMode() override;
        virtual const IIeee80211Mode *getModeForUnicastDataOrMgmtFrame(Ieee80211DataOrMgmtFrame *frame) override;
        virtual const IIeee80211Mode *getModeForMulticastDataOrMgmtFrame(Ieee80211DataOrMgmtFrame *frame) override;
        virtual const IIeee80211Mode *getModeForControlFrame(Ieee80211Frame *controlFrame) override;
        virtual const IIeee80211Mode* getResponseControlFrameMode() override;
};

} // namespace ieee80211
} // namespace inet

#endif
