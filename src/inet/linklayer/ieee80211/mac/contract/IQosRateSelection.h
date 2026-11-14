//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IQOSRATESELECTION_H
#define __INET_IQOSRATESELECTION_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/originator/TxopProcedure.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211TxVector.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211VhtCapabilities.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211RateSet.h"

namespace inet {
namespace ieee80211 {

/**
 * Abstract interface for rate selection. Rate selection decides what bit rate
 * (or MCS) should be used for any particular frame. The rules of rate selection
 * is described in the 802.11 specification in the section titled "Multirate Support".
 */
class INET_API IQosRateSelection
{
  public:
    virtual ~IQosRateSelection() {}

    // The frame requires a response. PREDICT uses its RA; RESPOND uses its TA.
    // PREDICT checks localRxPeerTx; RESPOND checks localTxPeerRx.
    virtual const physicallayer::IIeee80211Mode *computeResponseCtsFrameMode(Packet *packet, const Ptr<const Ieee80211RtsFrame>& rtsFrame, ResponseQueryRole role) = 0;
    virtual const physicallayer::IIeee80211Mode *computeResponseAckFrameMode(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader, ResponseQueryRole role) = 0;
    virtual const physicallayer::IIeee80211Mode *computeResponseBlockAckFrameMode(Packet *packet, const Ptr<const Ieee80211TwoAddressHeader>& blockAckReq, ResponseQueryRole role) = 0;
    // HCF derives startsTxop from the current frame sequence before this query.
    virtual const physicallayer::IIeee80211Mode *computeCompressedBlockAckReqMode(Packet *packet,
            const Ptr<const Ieee80211CompressedBlockAckReq>& blockAckReq, bool startsTxop) = 0;

    virtual const physicallayer::IIeee80211Mode *computeMode(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, TxopProcedure *txopProcedure) = 0;
    virtual std::pair<const physicallayer::IIeee80211Mode *, const physicallayer::IIeee80211Mode *> computeMeshPeerModes(
        MacAddress peer, const Ieee80211RateSetState& rates, const Ieee80211PeerPhyCapabilities& capabilities) = 0;
    virtual physicallayer::Ieee80211TxVector computeResponseTxVector(Packet *elicitingPacket,
        const physicallayer::IIeee80211Mode *responseMode, ResponseQueryRole role) const = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
