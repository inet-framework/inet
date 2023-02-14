//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCALARMEDIUMANALOGMODEL_H
#define __INET_SCALARMEDIUMANALOGMODEL_H

#include "inet/physicallayer/wireless/common/analogmodel/scalar/ScalarNoise.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/AnalogModelBase.h"

namespace inet {

namespace physicallayer {

class INET_API ScalarMediumAnalogModel : public AnalogModelBase
{
  protected:
    bool ignorePartialInterference;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual bool areOverlappingBands(Hz centerFrequency1, Hz bandwidth1, Hz centerFrequency2, Hz bandwidth2) const;
    virtual void addReception(const IReception *reception, simtime_t& noiseStartTime, simtime_t& noiseEndTime, std::map<simtime_t, W>& powerChanges) const;
    virtual void addNoise(const ScalarNoise *noise, simtime_t& noiseStartTime, simtime_t& noiseEndTime, std::map<simtime_t, W>& powerChanges) const;

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual W computeReceptionPower(const IRadio *radio, const ITransmission *transmission, const IArrival *arrival) const;
    virtual const INoise *computeNoise(const IListening *listening, const IInterference *interference) const override;
    virtual const INoise *computeNoise(const IReception *reception, const INoise *noise) const override;
    virtual const ISnir *computeSNIR(const IReception *reception, const INoise *noise) const override;
    virtual const IReception *computeReception(const IRadio *radio, const ITransmission *transmission, const IArrival *arrival) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

