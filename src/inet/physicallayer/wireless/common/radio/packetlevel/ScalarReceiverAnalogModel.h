//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCALARRECEIVERANALOGMODEL_H
#define __INET_SCALARRECEIVERANALOGMODEL_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ReceiverAnalogModelBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReceiverAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/INewReceptionAnalogModel.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/BandListening.h"

namespace inet {

namespace physicallayer {

class INET_API ScalarReceptionAnalogModel : public INewReceptionAnalogModel
{
  public:
    //Hz centerFrequency = Hz(NaN);
    //Hz bandwidth = Hz(NaN);
    //W power = W(NaN);

  public:
    ScalarReceptionAnalogModel(Hz centerFrequency, Hz bandwidth, W power) /* : centerFrequency(centerFrequency), bandwidth(bandwidth), power(power) */ {}
};

class INET_API ScalarReceiverAnalogModel : public ReceiverAnalogModelBase, public IReceiverAnalogModel
{
  protected:
    Hz centerFrequency = Hz(NaN);
    Hz bandwidth = Hz(NaN);
    // sensitivity?

  protected:
    virtual void initialize(int stage) override {
        if (stage == INITSTAGE_LOCAL) {
            centerFrequency = Hz(par("centerFrequency"));
            bandwidth = Hz(par("bandwidth"));
        }
    }

  public:
    virtual IListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition) const override {
        return new BandListening(radio, startTime, endTime, startPosition, endPosition, centerFrequency, bandwidth);

    }
};

} // namespace physicallayer

} // namespace inet

#endif

