//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PANELANTENNA_H
#define __INET_PANELANTENNA_H

#include "inet/physicallayer/wireless/common/base/packetlevel/AntennaBase.h"

namespace inet {

namespace cellular {

using namespace inet::physicallayer;

/**
 * A dummy base station panel antenna: a sector element pattern in the horizontal plane, and a
 * set of fixed beams on top of it. The radios of one band share one panel (see
 * SharedAntennaRef), so that a beam decision holds for all of them.
 */
class INET_API PanelAntenna : public AntennaBase
{
  protected:
    class INET_API Gain : public IAntennaGain {
      protected:
        double maxElementGain;
        double frontToBackRatio;
        deg halfPowerBeamwidth;
        deg boresight;
        // the beam, or NaN steering for the element pattern alone
        deg steering;
        deg beamWidth;
        double maxBeamGain;
        double sideLobeRatio;

      public:
        Gain(double maxElementGain, double frontToBackRatio, deg halfPowerBeamwidth, deg boresight, deg steering, deg beamWidth, double maxBeamGain, double sideLobeRatio);

        virtual double getMaxGain() const override;
        virtual double getMinGain() const override;
        virtual double computeGain(const Quaternion& direction) const override;
    };

    Ptr<const Gain> elementGain;
    std::vector<Ptr<const Gain>> beamGains;
    std::vector<deg> beamSteerings;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual Ptr<const IAntennaGain> getGain() const override { return elementGain; }

    // workaround for C1 (beam-addressable antenna): IAntenna has only the current gain
    int getNumBeams() const { return beamGains.size(); }
    Ptr<const IAntennaGain> getBeamGain(int beam) const { return beamGains.at(beam); }

    // the beam that points closest to a position, which is the ideal beam selection of the MAC
    int selectBeam(const Coord& position) const;
};

} // namespace cellular

} // namespace inet

#endif

