//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QOSRATESELECTION_H
#define __INET_QOSRATESELECTION_H

#include "inet/linklayer/ieee80211/mac/contract/IQosRateSelection.h"
#include "inet/linklayer/ieee80211/mac/rateselection/RateSelectionBase.h"

namespace inet {
namespace ieee80211 {

// IEEE Std 802.11-2024, 10.6.5 and 10.6.6: supported SU data, management, and control rate rules.
class INET_API QosRateSelection : public IQosRateSelection, public RateSelectionBase
{
  protected:
    // originator frame modes
    const physicallayer::IIeee80211Mode *multicastFrameMode = nullptr;
    const physicallayer::IIeee80211Mode *dataFrameMode = nullptr;
    const physicallayer::IIeee80211Mode *mgmtFrameMode = nullptr;
    const physicallayer::IIeee80211Mode *controlFrameMode = nullptr;

    const physicallayer::IIeee80211Mode *responseAckFrameMode = nullptr;
    const physicallayer::IIeee80211Mode *responseCtsFrameMode = nullptr;
    const physicallayer::IIeee80211Mode *responseBlockAckFrameMode = nullptr;

    // per-receiver unicast data-frame modes, resolved lazily from dataFrameBitratePerReceiver
    std::map<MacAddress, const physicallayer::IIeee80211Mode *> perReceiverDataFrameMode;
    bool perReceiverResolved = false;

  protected:
    virtual void initialize(int stage) override;

    // Builds perReceiverDataFrameMode on first use. Deferred out of initialize() because peer
    // MAC addresses are assigned during INITSTAGE_LINK_LAYER with undefined intra-stage module
    // ordering; the first transmitted data frame occurs after all init stages, so this is race-free.
    virtual void ensurePerReceiverModesResolved();

    virtual const physicallayer::IIeee80211Mode *getMode(Packet *packet, const Ptr<const Ieee80211MacHeader>& header);
    virtual const physicallayer::IIeee80211Mode *computeControlFrameMode(const Ptr<const Ieee80211MacHeader>& header, bool startsTxop, const physicallayer::IIeee80211Mode *previousModeForReceiver);
    virtual const physicallayer::IIeee80211Mode *computeDataOrMgmtFrameMode(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader, bool useFastestMode);

  public:
    // A control response frame is a control frame that is transmitted as a response to the reception of a frame a SIFS
    // time after the PPDU containing the frame that elicited the response, e.g. a CTS in response to an RTS
    // reception, an ACK in response to a DATA reception, a BlockAck in response to a BlockAckReq reception. In
    // some situations, the transmission of a control frame is not a control response transmission, such as when a CTS
    // is used to initiate a TXOP.
    virtual const physicallayer::IIeee80211Mode *computeResponseMode(
            const physicallayer::IIeee80211Mode *elicitingMode,
            Ieee80211ResponseFrameKind responseKind, const MacAddress& receiver) override;
    virtual const physicallayer::IIeee80211Mode *computeResponseCtsFrameMode(Packet *packet, const Ptr<const Ieee80211RtsFrame>& rtsFrame) override;
    virtual const physicallayer::IIeee80211Mode *computeResponseAckFrameMode(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) override;
    virtual const physicallayer::IIeee80211Mode *computeResponseBlockAckFrameMode(Packet *packet, const Ptr<const Ieee80211BlockAckReq>& blockAckReq) override;

    virtual const physicallayer::IIeee80211Mode *computeMode(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, bool startsTxop, const physicallayer::IIeee80211Mode *previousModeForReceiver, bool useFastestMode = false) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif
