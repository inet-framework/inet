/* -*- mode:c++ -*- ********************************************************
 * author:      Jerome Rousselot
 *
 * copyright:   (C) 2008 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
 *              Real-Time Software and Networking
 *              Jaquet-Droz 1, CH-2002 Neuchatel, Switzerland.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 * description: this AnalogueModel models free-space pathloss
 ***************************************************************************/

#include "inet/physicallayer/pathloss/UWBIRStochasticPathLoss.h"

namespace inet {

namespace physicallayer {

const Hz UWBIRStochasticPathLoss::fc = MHz(4492.8); // mandatory band 3, center frequency
const m UWBIRStochasticPathLoss::d0 = m(1);
const double UWBIRStochasticPathLoss::kappa = 1;
const double UWBIRStochasticPathLoss::n1_limit = 1.25;
const double UWBIRStochasticPathLoss::n2_limit = 2;
const double UWBIRStochasticPathLoss::n3_limit = 2;

UWBIRStochasticPathLoss::UWBIRStochasticPathLoss() :
    PL0(0),
    muGamma(0),
    muSigma(0),
    sigmaGamma(0),
    sigmaSigma(0),
    pathloss_exponent(0.0),
    shadowing(true)
{
}

double UWBIRStochasticPathLoss::simtruncnormal(double mean, double stddev, double a, int /*rng*/) const
{
    double result = a + 1;
    while (result > a || result < -a)
      result = normal(mean, stddev, 0);
    return result;
}

void UWBIRStochasticPathLoss::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        PL0 = par("PL0");
        muGamma = par("muGamma");
        muSigma = par("muSigma");
        sigmaGamma = par("sigmaGamma");
        sigmaSigma = par("sigmaSigma");
        shadowing = par("shadowing");
    }
}

std::ostream& UWBIRStochasticPathLoss::printToStream(std::ostream& stream, int level) const
{
    stream << "UWBIRStochasticPathLoss";
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", PL0 = " << PL0
               << ", muGamma = " << muGamma
               << ", muSigma = " << muSigma
               << ", sigmaGamma = " << sigmaGamma
               << ", sigmaSigma = " << sigmaSigma
               << ", shadowing = " << shadowing;
    return stream;
}

double UWBIRStochasticPathLoss::getFDPathLoss(Hz frequency, m distance) const {
    return 0.5 * PL0 * pow(unit(frequency / fc).get(), -2 * (kappa + 1)) / pow(unit(distance / d0).get(), pathloss_exponent);
}

double UWBIRStochasticPathLoss::getNarrowBandFreeSpacePathLoss(Hz frequency, m distance) const
{
    double attenuation = 4 * M_PI * unit(distance * frequency / mps(SPEED_OF_LIGHT)).get();
    return 1.0 / (attenuation * attenuation);
}

double UWBIRStochasticPathLoss::getGhassemzadehPathLoss(double gamma, double S, m distance) const
{
    double attenuation = PL0;
    if (distance < d0)
        distance = d0;
    attenuation = attenuation - 10 * gamma * log10(unit(distance / d0).get());
    if (shadowing)
        attenuation = attenuation - S;
    return math::dB2fraction(attenuation);
}

double UWBIRStochasticPathLoss::computePathLoss(mps propagationSpeed, Hz frequency, m distance) const
{
    double n1 = simtruncnormal(0, 1, n1_limit, 1);
    double n2 = simtruncnormal(0, 1, n2_limit, 2);
    double n3 = simtruncnormal(0, 1, n3_limit, 3);
    double gamma = muGamma + n1 * sigmaGamma;
    double sigma = muSigma + n3 * sigmaSigma;
    double S = n2 * sigma;
    return getGhassemzadehPathLoss(gamma, S, distance);
}

} // namespace physicallayer

} // namespace inet
