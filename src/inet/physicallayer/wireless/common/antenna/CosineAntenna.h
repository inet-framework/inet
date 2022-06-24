//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_COSINEANTENNA_H
#define __INET_COSINEANTENNA_H

#include "inet/physicallayer/wireless/common/base/packetlevel/AntennaBase.h"

namespace inet {

namespace physicallayer {

class INET_API CosineAntenna : public AntennaBase
{
  protected:
    virtual void initialize(int stage) override;

    class INET_API AntennaGain : public IAntennaGain {
      public:
        AntennaGain(double maxGain, deg beamWidth);
        virtual double getMinGain() const override { return 0; }
        virtual double getMaxGain() const override { return maxGain; }
        virtual deg getBeamWidth() const { return beamWidth; }
        virtual double computeGain(const Quaternion& direction) const override;

      protected:
        double maxGain;
        deg beamWidth;
    };

    Ptr<AntennaGain> gain;

  public:
    CosineAntenna();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual Ptr<const IAntennaGain> getGain() const override { return gain; }
};

} // namespace physicallayer

} // namespace inet

#endif

