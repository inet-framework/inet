//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INTERPOLATINGANTENNA_H
#define __INET_INTERPOLATINGANTENNA_H

#include "inet/physicallayer/wireless/common/base/packetlevel/AntennaBase.h"

namespace inet {

namespace physicallayer {

/**
 * TODO refactor to use Delaunay triangulation on unit sphere, i.e. triangulate
 * result from enclosing spherical triangle as seen from the center
 */
class INET_API InterpolatingAntenna : public AntennaBase
{
  protected:
    class INET_API AntennaGain : public IAntennaGain {
      protected:
        double minGain;
        double maxGain;
        std::map<rad, double> elevationGainMap;
        std::map<rad, double> headingGainMap;
        std::map<rad, double> bankGainMap;

      protected:
        virtual void parseMap(std::map<rad, double>& gainMap, const char *text);
        virtual double computeGain(const std::map<rad, double>& gainMap, rad angle) const;

      public:
        AntennaGain(const char *elevation, const char *heading, const char *bank);
        virtual double getMinGain() const override { return minGain; }
        virtual double getMaxGain() const override { return maxGain; }
        virtual double computeGain(const Quaternion& direction) const override;
    };

    Ptr<AntennaGain> gain;

  protected:
    virtual void initialize(int stage) override;

  public:
    InterpolatingAntenna();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual Ptr<const IAntennaGain> getGain() const override { return gain; }
};

} // namespace physicallayer

} // namespace inet

#endif

