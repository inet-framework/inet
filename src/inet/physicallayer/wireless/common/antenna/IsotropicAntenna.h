//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ISOTROPICANTENNA_H
#define __INET_ISOTROPICANTENNA_H

#include "inet/physicallayer/wireless/common/base/packetlevel/AntennaBase.h"

namespace inet {

namespace physicallayer {

class INET_API IsotropicAntenna : public AntennaBase
{
  protected:
    class INET_API AntennaGain : public IAntennaGain {
      public:
        virtual double getMinGain() const override { return 1; }
        virtual double getMaxGain() const override { return 1; }
        virtual double computeGain(const Quaternion& direction) const override { return 1; }
    };

    Ptr<AntennaGain> gain;

  public:
    IsotropicAntenna();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual Ptr<const IAntennaGain> getGain() const override { return gain; }
};

} // namespace physicallayer

} // namespace inet

#endif

