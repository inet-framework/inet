//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PARABOLICANTENNA_H
#define __INET_PARABOLICANTENNA_H

#include "inet/physicallayer/wireless/common/base/packetlevel/AntennaBase.h"

namespace inet {

namespace physicallayer {

class INET_API ParabolicAntenna : public AntennaBase
{
  protected:
    class INET_API AntennaGain : public IAntennaGain {
      public:
        AntennaGain(double maxGain, double minGain, deg beamWidth);
        virtual double getMaxGain() const override { return maxGain; }
        virtual double getMinGain() const override { return minGain; }
        virtual deg getBeamWidth() const { return beamWidth; }
        virtual double computeGain(const Quaternion& direction) const override;

      protected:
        double maxGain;
        double minGain;
        deg beamWidth;
    };

    Ptr<AntennaGain> gain;

  protected:
    virtual void initialize(int stage) override;

  public:
    ParabolicAntenna();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual Ptr<const IAntennaGain> getGain() const override { return gain; }
};

} // namespace physicallayer

} // namespace inet

#endif

