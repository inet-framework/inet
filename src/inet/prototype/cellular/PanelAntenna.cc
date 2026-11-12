//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/prototype/cellular/PanelAntenna.h"

#include "inet/mobility/contract/IMobility.h"

namespace inet {

namespace cellular {

Define_Module(PanelAntenna);

static deg normalizeAngle(deg angle)
{
    double degrees = std::fmod(angle.get() + 180, 360);
    if (degrees < 0)
        degrees += 360;
    return deg(degrees - 180);
}

// the azimuth of a direction given as the rotation from the X axis
static deg computeAzimuth(const Quaternion& direction)
{
    Coord vector = direction.rotate(Coord::X_AXIS);
    return rad(std::atan2(vector.y, vector.x));
}

void PanelAntenna::initialize(int stage)
{
    AntennaBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        double maxElementGain = math::dB2fraction(par("maxElementGain"));
        double frontToBackRatio = math::dB2fraction(par("frontToBackRatio"));
        deg halfPowerBeamwidth = deg(par("halfPowerBeamwidth"));
        deg boresight = deg(par("boresight"));
        deg beamWidth = deg(par("beamWidth"));
        double maxBeamGain = math::dB2fraction(par("maxBeamGain"));
        double sideLobeRatio = math::dB2fraction(par("sideLobeRatio"));
        int numBeams = par("numBeams");
        deg beamSpan = deg(par("beamSpan"));
        elementGain = makeShared<Gain>(maxElementGain, frontToBackRatio, halfPowerBeamwidth, boresight, deg(NaN), beamWidth, 1, 1);
        for (int i = 0; i < numBeams; i++) {
            // the beams are spread evenly over the span, centered on the boresight
            deg steering = numBeams == 1 ? deg(0) : beamSpan * (-0.5 + (double)i / (numBeams - 1));
            beamSteerings.push_back(steering);
            beamGains.push_back(makeShared<Gain>(maxElementGain, frontToBackRatio, halfPowerBeamwidth, boresight, steering, beamWidth, maxBeamGain, sideLobeRatio));
        }
    }
}

std::ostream& PanelAntenna::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "PanelAntenna";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(numBeams, beamGains.size());
    return AntennaBase::printToStream(stream, level);
}

int PanelAntenna::selectBeam(const Coord& position) const
{
    if (beamGains.empty())
        return -1;
    Coord direction = position - mobility->getCurrentPosition();
    Quaternion localDirection = Quaternion(mobility->getCurrentAngularPosition()).inverse() * Quaternion::rotationFromTo(Coord::X_AXIS, direction);
    deg azimuth = normalizeAngle(computeAzimuth(localDirection) - deg(par("boresight")));
    int bestBeam = 0;
    for (int i = 1; i < (int)beamSteerings.size(); i++)
        if (std::abs((azimuth - beamSteerings[i]).get()) < std::abs((azimuth - beamSteerings[bestBeam]).get()))
            bestBeam = i;
    return bestBeam;
}

PanelAntenna::Gain::Gain(double maxElementGain, double frontToBackRatio, deg halfPowerBeamwidth, deg boresight, deg steering, deg beamWidth, double maxBeamGain, double sideLobeRatio) :
    maxElementGain(maxElementGain), frontToBackRatio(frontToBackRatio), halfPowerBeamwidth(halfPowerBeamwidth), boresight(boresight),
    steering(steering), beamWidth(beamWidth), maxBeamGain(maxBeamGain), sideLobeRatio(sideLobeRatio)
{
}

double PanelAntenna::Gain::getMaxGain() const
{
    return maxElementGain * maxBeamGain;
}

double PanelAntenna::Gain::getMinGain() const
{
    return maxElementGain / frontToBackRatio * maxBeamGain / sideLobeRatio;
}

double PanelAntenna::Gain::computeGain(const Quaternion& direction) const
{
    // the parabolic main lobe of TR 38.901 Table 7.3-1 in the horizontal plane, cut at the floor
    auto computeLobe = [] (deg offset, deg width, double floor) {
        double attenuation = math::dB2fraction(12 * std::pow((offset / width).get(), 2));
        return 1 / std::min(attenuation, floor);
    };
    deg azimuth = normalizeAngle(computeAzimuth(direction) - boresight);
    // workaround for C17 (co-located antennas): the analog model gives the direction between
    // two antennas at the same position, the sectors of one site, as a NaN rotation
    if (std::isnan(azimuth.get()))
        return getMinGain();
    double gain = maxElementGain * computeLobe(azimuth, halfPowerBeamwidth, frontToBackRatio);
    if (!std::isnan(steering.get()))
        gain *= maxBeamGain * computeLobe(normalizeAngle(azimuth - steering), beamWidth, sideLobeRatio);
    return gain;
}

} // namespace cellular

} // namespace inet

