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

#ifndef __INET_IQOSRATESELECTION_H
#define __INET_IQOSRATESELECTION_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/ieee80211/mac/originator/TxopProcedure.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"

using namespace inet::physicallayer;

namespace inet {
namespace ieee80211 {

/**
 * Abstract interface for rate selection. Rate selection decides what bit rate
 * (or MCS) should be used for any particular frame. The rules of rate selection
 * is described in the 802.11 specification in the section titled "Multirate Support".
 */
class INET_API IQoSRateSelection
{
    public:
        virtual ~IQoSRateSelection() {}

        virtual const IIeee80211Mode *computeResponseCtsFrameMode(Ieee80211RTSFrame *rtsFrame) = 0;
        virtual const IIeee80211Mode *computeResponseAckFrameMode(Ieee80211DataOrMgmtFrame *dataOrMgmtFrame) = 0;
        virtual const IIeee80211Mode *computeResponseBlockAckFrameMode(Ieee80211BlockAckReq *blockAckReq) = 0;

        virtual const IIeee80211Mode *computeMode(Ieee80211Frame *frame, TxopProcedure *txopProcedure) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
