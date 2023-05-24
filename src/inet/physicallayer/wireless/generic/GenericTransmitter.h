//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GENERICTRANSMITTER_H
#define __INET_GENERICTRANSMITTER_H

#include "inet/physicallayer/wireless/common/base/packetlevel/TransmitterBase.h"
#include "inet/physicallayer/wireless/common/analogmodel/unitdisk/UnitDiskTransmitterAnalogModel.h"

namespace inet {

namespace physicallayer {

/**
 * Implements the GenericTransmitter model, see the NED file for details.
 */
class INET_API GenericTransmitter : public TransmitterBase
{
  protected:
    simtime_t preambleDuration = -1;
    b headerLength = b(-1);
    bps bitrate = bps(NaN);

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const ITransmission *createTransmission(const IRadio *radio, const Packet *packet, const simtime_t startTime) const override;
    virtual simtime_t getPreambleDuration() const { return preambleDuration; }
    virtual b getHeaderLength() const { return headerLength; }
    virtual bps getBitrate() const { return bitrate; }

    virtual m getMaxCommunicationRange() const override {
        // TODO should we move this to the base class? how is it not going to be repeated in other transmitters?
        if (auto analogModel = dynamic_cast<UnitDiskTransmitterAnalogModel *>(getAnalogModel()))
            return analogModel->getCommunicationRange();
        else
            return TransmitterBase::getMaxCommunicationRange();
    }

    virtual m getMaxInterferenceRange() const override {
        // TODO should we move this to the base class? how is it not going to be repeated in other transmitters?
        if (auto analogModel = dynamic_cast<UnitDiskTransmitterAnalogModel *>(getAnalogModel()))
            return analogModel->getInterferenceRange();
        else
            return TransmitterBase::getMaxInterferenceRange();
    }
};

} // namespace physicallayer

} // namespace inet

#endif

