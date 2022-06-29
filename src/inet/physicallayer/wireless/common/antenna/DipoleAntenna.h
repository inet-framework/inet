//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DIPOLEANTENNA_H
#define __INET_DIPOLEANTENNA_H

#include "inet/physicallayer/wireless/common/base/packetlevel/AntennaBase.h"

namespace inet {

namespace physicallayer {

class INET_API DipoleAntenna : public AntennaBase
{
  protected:
    virtual void initialize(int stage) override;

    class INET_API AntennaGain : public IAntennaGain {
      protected:
        Coord wireAxisDirection;
        m length;

      public:
        AntennaGain(const char *wireAxis, m length);
        virtual m getLength() const { return length; }
        virtual double getMinGain() const override { return 0; }
        virtual double getMaxGain() const override { return 1.5; }
        virtual double computeGain(const Quaternion& direction) const override;
    };

    Ptr<AntennaGain> gain;

  public:
    DipoleAntenna();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual Ptr<const IAntennaGain> getGain() const override { return gain; }
};

} // namespace physicallayer

} // namespace inet

#endif

