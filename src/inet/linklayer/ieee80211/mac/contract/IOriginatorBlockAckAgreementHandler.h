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

#ifndef __INET_IORIGINATORBLOCKACKAGREEMENTHANDLER_H
#define __INET_IORIGINATORBLOCKACKAGREEMENTHANDLER_H

#include "inet/linklayer/ieee80211/mac/contract/IOriginatorBlockAckAgreementPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IBlockAckAgreementHandlerCallback.h"
#include "inet/linklayer/ieee80211/mac/contract/IProcedureCallback.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/common/MACAddress.h"

namespace inet {
namespace ieee80211 {

class INET_API IOriginatorBlockAckAgreementHandler
{
    public:
        virtual ~IOriginatorBlockAckAgreementHandler() { }

        virtual void processReceivedBlockAck(Ieee80211BlockAck *blockAck, IBlockAckAgreementHandlerCallback *callback) = 0;
        virtual void processTransmittedAddbaReq(Ieee80211AddbaRequest *addbaReq) = 0;
        virtual void processTransmittedDataFrame(Ieee80211DataFrame *dataFrame, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *callback) = 0;
        virtual void processReceivedAddbaResp(Ieee80211AddbaResponse *addbaResp, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback) = 0;
        virtual void processReceivedDelba(Ieee80211Delba *delba, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy) = 0;
        virtual void processTransmittedDelba(Ieee80211Delba *delba) = 0;
        virtual void blockAckAgreementExpired(IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback) = 0;

        virtual OriginatorBlockAckAgreement *getAgreement(MACAddress receiverAddr, Tid tid) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
