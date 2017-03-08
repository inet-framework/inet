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

#ifndef __INET_QOSRATESELECTION_H
#define __INET_QOSRATESELECTION_H

#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/IQoSRateSelection.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateControl.h"

namespace inet {
namespace ieee80211 {

/*
 * This class implements the following subclauses:
 *
 * 9.7.5 Rate selection for data and management frames
 *      9.7.5.3 Rate selection for other group addressed data and management frames
 *      9.7.5.6 Rate selection for other data and management frames
 *
 * 9.7.6 Rate selection for control frames
 *      TODO: Control frames carried in an A-MPDU shall be sent at a rate selected from the rules defined in 9.7.5.6.
 *      9.7.6.2 Rate selection for control frames that initiate a TXOP
 *      9.7.6.4 Rate selection for control frames that are not control response frames
 *      9.7.6.5 Rate selection for control response frames
 */
class INET_API QoSRateSelection : public IQoSRateSelection, public ModeSetListener
{
    protected:
        IRateControl *dataOrMgmtRateControl = nullptr;

        const Ieee80211ModeSet *modeSet = nullptr;
        std::map<MACAddress, const IIeee80211Mode *> lastTransmittedFrameMode;

        // originator frame modes
        const IIeee80211Mode *multicastFrameMode = nullptr;
        const IIeee80211Mode *fastestMandatoryMode = nullptr;

        const IIeee80211Mode *dataFrameMode = nullptr;
        const IIeee80211Mode *mgmtFrameMode = nullptr;
        const IIeee80211Mode *controlFrameMode = nullptr;

        const IIeee80211Mode *responseAckFrameMode = nullptr;
        const IIeee80211Mode *responseCtsFrameMode = nullptr;
        const IIeee80211Mode *responseBlockAckFrameMode = nullptr;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;
        virtual void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details) override;

        virtual const IIeee80211Mode *getMode(Ieee80211Frame *frame);
        virtual const IIeee80211Mode *computeControlFrameMode(Ieee80211Frame *frame, TxopProcedure *txopProcedure);
        virtual const IIeee80211Mode *computeDataOrMgmtFrameMode(Ieee80211DataOrMgmtFrame *dataOrMgmtFrame);

        virtual bool isControlResponseFrame(Ieee80211Frame *frame, TxopProcedure *txopProcedure);

    public:
        // A control response frame is a control frame that is transmitted as a response to the reception of a frame a SIFS
        // time after the PPDU containing the frame that elicited the response, e.g. a CTS in response to an RTS
        // reception, an ACK in response to a DATA reception, a BlockAck in response to a BlockAckReq reception. In
        // some situations, the transmission of a control frame is not a control response transmission, such as when a CTS
        // is used to initiate a TXOP.
        virtual const IIeee80211Mode *computeResponseCtsFrameMode(Ieee80211RTSFrame *rtsFrame) override;
        virtual const IIeee80211Mode *computeResponseAckFrameMode(Ieee80211DataOrMgmtFrame *dataOrMgmtFrame) override;
        virtual const IIeee80211Mode *computeResponseBlockAckFrameMode(Ieee80211BlockAckReq *blockAckReq) override;

        virtual const IIeee80211Mode *computeMode(Ieee80211Frame* frame, TxopProcedure *txopProcedure) override;

        virtual void frameTransmitted(Ieee80211Frame *frame);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_QOSRATESELECTION_H
