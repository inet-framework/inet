//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FLATRECEIVERBASE_H
#define __INET_FLATRECEIVERBASE_H

#include <optional>

#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandReceiverBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IErrorModel.h"

namespace inet {

namespace physicallayer {

// REFACTOR TODO
class INET_API FlatReceiverBase : public NarrowbandReceiverBase
{
  protected:
    const IErrorModel *errorModel;
    W energyDetection;
    W sensitivity;

  protected:
    virtual void initialize(int stage) override;

    using NarrowbandReceiverBase::computeIsReceptionPossible;

    virtual Packet *computeReceivedPacket(const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const override;

    /** @name Regions of the signal parts */
    //@{
    /**
     * The bits of the header part or the data part of a transmission that the
     * error decisions divide into regions.
     */
    struct PartRange
    {
        // the number of bits of the part
        b length;
        // false if the transmitted packet does not hold the bits of the part as content
        bool isInPacket;
        // the offset of the bits in the transmitted packet, if the packet holds them
        b offset;
    };

    /**
     * Returns the bits of the header part or the data part of the transmission.
     * The header comes before the data in the packet. Returns no value if the
     * packet does not hold the complete part, for example if a length field
     * gives more bits than the packet has. This implementation throws an error:
     * a receiver that decides the regions of its parts must name them.
     */
    virtual std::optional<PartRange> computePartRange(const ITransmission *transmission, IRadioSignal::SignalPart part) const;

    /**
     * Returns the start and the end of the time in which the transmitter sends
     * the bits [offset, offset + length) of a part that has partLength bits.
     * This implementation divides the time of the part in proportion to the
     * bits.
     */
    virtual std::pair<simtime_t, simtime_t> computeRegionInterval(const IReception *reception, IRadioSignal::SignalPart part, b offset, b length, b partLength) const;

    /**
     * Decides each region of the header part or the data part one time, and
     * returns true if no region failed. A region fails if the reception of its
     * time interval is not successful, that is, its SNIR is not above the
     * threshold, or else if one draw against its error rate fails. Both use the
     * SNIR of the time of the region. A region is the whole part in the packet
     * corruption mode, and one chunk of the part in the other modes; a part that
     * the packet does not hold is one region. The function adds the failed
     * regions that the packet holds to errorRegions.
     */
    virtual bool computeIsPartSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir, std::vector<ErrorRegion>& errorRegions) const;
    //@}

  public:
    FlatReceiverBase();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual W getMinReceptionPower() const override { return sensitivity; }

    virtual const IListeningDecision *computeListeningDecision(const IListening *listening, const IInterference *interference) const override;
    virtual const IReceptionDecision *computeReceptionDecision(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const override;
    virtual const IReceptionResult *computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const override;

    virtual const IErrorModel *getErrorModel() const { return errorModel; }

    virtual W getEnergyDetection() const { return energyDetection; }
    virtual void setEnergyDetection(W energyDetection) { this->energyDetection = energyDetection; }

    virtual W getSensitivity() const { return sensitivity; }
    virtual void setSensitivity(W sensitivity) { this->sensitivity = sensitivity; }
};

} // namespace physicallayer

} // namespace inet

#endif

