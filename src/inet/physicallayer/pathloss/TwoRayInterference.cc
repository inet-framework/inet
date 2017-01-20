#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/pathloss/TwoRayInterference.h"

namespace inet {
namespace physicallayer {

Define_Module(TwoRayInterference);

TwoRayInterference::TwoRayInterference() :
    epsilon_r(1.0)
{
}

void TwoRayInterference::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        epsilon_r = par("epsilon_r");
    }
}

std::ostream& TwoRayInterference::printToStream(std::ostream& os, int level) const
{
    os << "TwoRayInterference";
    if (level >= PRINT_LEVEL_TRACE)
        os << ", epsilon_r = " << epsilon_r;
    return os;
}

double TwoRayInterference::computePathLoss(const ITransmission* transmission, const IArrival* arrival) const
{
    auto radioMedium = transmission->getTransmitter()->getMedium();
    auto narrowbandSignalAnalogModel = check_and_cast<const INarrowbandSignal *>(transmission->getAnalogModel());
    mps propagationSpeed = radioMedium->getPropagation()->getPropagationSpeed();
    Hz carrierFrequency = Hz(narrowbandSignalAnalogModel->getCarrierFrequency());
    const m waveLength = propagationSpeed / carrierFrequency;

    return computeTwoRayInterference(transmission->getStartPosition(), arrival->getStartPosition(), waveLength);
}

double TwoRayInterference::computeTwoRayInterference(const Coord& pos_t, const Coord& pos_r, m lambda) const
{
    const m distance { pos_r.distance(pos_t) };
    const m h_sum { pos_t.z + pos_r.z };
    const m h_dif { pos_t.z - pos_r.z };
    const m d_los = sqrt(distance * distance + h_dif * h_dif);
    const m d_ref = sqrt(distance * distance + h_sum * h_sum);

    const double phi = 2 * M_PI * unit((d_los - d_ref) / lambda).get();
    const double sin_theta = unit(h_sum / d_ref).get();
    const double cos_theta = unit(distance / d_ref).get();
    const double gamma = sqrt(epsilon_r - cos_theta * cos_theta);
    const double Gamma = (sin_theta - gamma) / (sin_theta + gamma);

    const double space = 4 * M_PI * unit(distance / lambda).get();
    const double rays = sqrt(pow(1 + Gamma * cos(phi), 2) + Gamma * Gamma * pow(sin(phi), 2));
    return pow(space / rays, -2);
}

double TwoRayInterference::computePathLoss(mps propagationSpeed, Hz frequency, m distance) const
{
    return NaN;
}

m TwoRayInterference::computeRange(mps propagationSpeed, Hz frequency, double loss) const
{
    return m(NaN);
}

} // namespace physicallayer
} // namespace artery
