//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CONSTANTGAINANTENNA_H
#define __INET_CONSTANTGAINANTENNA_H

#include "inet/physicallayer/wireless/common/base/packetlevel/AntennaBase.h"

namespace inet {

namespace physicallayer {

class INET_API ConstantGainAntenna : public AntennaBase
{
  protected:
    virtual void initialize(int stage) override;

    class INET_API AntennaGain : public IAntennaGain {
      public:
        AntennaGain(double gain) : gain(gain) {}
        virtual double getMinGain() const override { return gain; }
        virtual double getMaxGain() const override { return gain; }
        virtual double computeGain(const Quaternion& direction) const override { return gain; }

      protected:
        double gain;
    };

    Ptr<AntennaGain> gain;

  public:
    ConstantGainAntenna();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual Ptr<const IAntennaGain> getGain() const override { return gain; }
};

} // namespace physicallayer

} // namespace inet

#endif

