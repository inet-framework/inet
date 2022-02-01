//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QOSRATESELECTION_H
#define __INET_QOSRATESELECTION_H

#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/IQosRateSelection.h"
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
 *      TODO Control frames carried in an A-MPDU shall be sent at a rate selected from the rules defined in 9.7.5.6.
 *      9.7.6.2 Rate selection for control frames that initiate a TXOP
 *      9.7.6.4 Rate selection for control frames that are not control response frames
 *      9.7.6.5 Rate selection for control response frames
 */
class INET_API QosRateSelection : public IQosRateSelection, public ModeSetListener
{
  protected:
    IRateControl *dataOrMgmtRateControl = nullptr;

    const physicallayer::Ieee80211ModeSet *modeSet = nullptr;
    std::map<MacAddress, const physicallayer::IIeee80211Mode *> lastTransmittedFrameMode;

    // originator frame modes
    const physicallayer::IIeee80211Mode *multicastFrameMode = nullptr;
    const physicallayer::IIeee80211Mode *fastestMandatoryMode = nullptr;

    const physicallayer::IIeee80211Mode *dataFrameMode = nullptr;
    const physicallayer::IIeee80211Mode *mgmtFrameMode = nullptr;
    const physicallayer::IIeee80211Mode *controlFrameMode = nullptr;

    const physicallayer::IIeee80211Mode *responseAckFrameMode = nullptr;
    const physicallayer::IIeee80211Mode *responseCtsFrameMode = nullptr;
    const physicallayer::IIeee80211Mode *responseBlockAckFrameMode = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    virtual const physicallayer::IIeee80211Mode *getMode(Packet *packet, const Ptr<const Ieee80211MacHeader>& header);
    virtual const physicallayer::IIeee80211Mode *computeControlFrameMode(const Ptr<const Ieee80211MacHeader>& header, TxopProcedure *txopProcedure);
    virtual const physicallayer::IIeee80211Mode *computeDataOrMgmtFrameMode(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader);

    virtual bool isControlResponseFrame(const Ptr<const Ieee80211MacHeader>& header, TxopProcedure *txopProcedure);

  public:
    // A control response frame is a control frame that is transmitted as a response to the reception of a frame a SIFS
    // time after the PPDU containing the frame that elicited the response, e.g. a CTS in response to an RTS
    // reception, an ACK in response to a DATA reception, a BlockAck in response to a BlockAckReq reception. In
    // some situations, the transmission of a control frame is not a control response transmission, such as when a CTS
    // is used to initiate a TXOP.
    virtual const physicallayer::IIeee80211Mode *computeResponseCtsFrameMode(Packet *packet, const Ptr<const Ieee80211RtsFrame>& rtsFrame) override;
    virtual const physicallayer::IIeee80211Mode *computeResponseAckFrameMode(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) override;
    virtual const physicallayer::IIeee80211Mode *computeResponseBlockAckFrameMode(Packet *packet, const Ptr<const Ieee80211BlockAckReq>& blockAckReq) override;

    virtual const physicallayer::IIeee80211Mode *computeMode(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, TxopProcedure *txopProcedure) override;

    virtual void frameTransmitted(Packet *packet, const Ptr<const Ieee80211MacHeader>& header);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

