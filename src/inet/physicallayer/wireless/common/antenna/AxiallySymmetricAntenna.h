//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_AXIALLYSYMMETRICANTENNA_H
#define __INET_AXIALLYSYMMETRICANTENNA_H

#include "inet/physicallayer/wireless/common/base/packetlevel/AntennaBase.h"

namespace inet {

namespace physicallayer {

class INET_API AxiallySymmetricAntenna : public AntennaBase
{
  protected:
    class INET_API AntennaGain : public IAntennaGain {
      protected:
        double minGain = NaN;
        double maxGain = NaN;
        Coord axisOfSymmetryDirection = Coord::NIL;
        std::map<rad, double> gainMap;

      public:
        AntennaGain(const char *axis, double baseGain, const char *gains);

        virtual double getMinGain() const override { return minGain; }
        virtual double getMaxGain() const override { return maxGain; }
        virtual double computeGain(const Quaternion& direction) const override;
    };

    Ptr<AntennaGain> gain;

  protected:
    virtual void initialize(int stage) override;

  public:
    AxiallySymmetricAntenna();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual Ptr<const IAntennaGain> getGain() const override { return gain; }
};

} // namespace physicallayer

} // namespace inet

#endif

