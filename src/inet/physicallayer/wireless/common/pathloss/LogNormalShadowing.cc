//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
/***************************************************************************
* author:      Andreas Kuntz
*
* copyright:   (c) 2008 Institute of Telematics, University of Karlsruhe (TH)
*
* author:      Alfonso Ariza
*              Malaga university
*
***************************************************************************/

#include "inet/physicallayer/wireless/common/pathloss/LogNormalShadowing.h"

#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/INarrowbandSignalAnalogModel.h"

namespace inet {

namespace physicallayer {

Define_Module(LogNormalShadowing);

LogNormalShadowing::LogNormalShadowing() :
    sigma(1)
{
}

void LogNormalShadowing::initialize(int stage)
{
    FreeSpacePathLoss::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        sigma = par("sigma");
        correlationDistance = m(par("correlationDistance"));
    }
}

std::ostream& LogNormalShadowing::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "LogNormalShadowing";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(alpha)
               << EV_FIELD(systemLoss)
               << EV_FIELD(sigma)
               << EV_FIELD(correlationDistance);
    return stream;
}

double LogNormalShadowing::computePathLoss(const ITransmission *transmission, const IArrival *arrival) const
{
    if (std::isnan(correlationDistance.get()))
        return PathLossBase::computePathLoss(transmission, arrival);
    auto key = std::make_pair(transmission->getTransmitterRadioId(), arrival->getReceiverRadio()->getId());
    const Coord& position = arrival->getStartPosition();
    auto it = linkShadowings.find(key);
    if (it == linkShadowings.end() || m(it->second.position.distance(position)) > correlationDistance)
        it = linkShadowings.insert_or_assign(key, LinkShadowing{position, normal(0.0, sigma)}).first;
    // the same reduction as PathLossBase::computePathLoss(transmission, arrival)
    auto analogModel = check_and_cast<const INarrowbandSignalAnalogModel *>(transmission->getAnalogModel());
    mps propagationSpeed = transmission->getMedium()->getPropagation()->getPropagationSpeed();
    Hz centerFrequency = Hz(analogModel->getCenterFrequency());
    m distance = m(arrival->getStartPosition().distance(transmission->getStartPosition()));
    return computePathLoss(propagationSpeed, centerFrequency, distance, it->second.shadowing);
}

double LogNormalShadowing::computePathLoss(mps propagationSpeed, Hz frequency, m distance) const
{
    return computePathLoss(propagationSpeed, frequency, distance, normal(0.0, sigma));
}

double LogNormalShadowing::computePathLoss(mps propagationSpeed, Hz frequency, m distance, double shadowing) const
{
    m d0 = m(1.0);
    // reference path loss
    double freeSpacePathLoss = computeFreeSpacePathLoss(propagationSpeed / frequency, d0, alpha, systemLoss);
    double PL_d0_db = 10.0 * log10(1 / freeSpacePathLoss);
    // path loss at distance d + the shadowing (dB)
    double PL_db = PL_d0_db + 10 * alpha * log10((distance / d0).get<unit>()) + shadowing;
    return math::dB2fraction(-PL_db);
}

} // namespace physicallayer

} // namespace inet

