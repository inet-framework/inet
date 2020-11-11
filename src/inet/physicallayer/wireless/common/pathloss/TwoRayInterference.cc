
#include "inet/physicallayer/wireless/common/pathloss/TwoRayInterference.h"

#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"

namespace inet {
namespace physicallayer {

Define_Module(TwoRayInterference);

// shortcut for squaring numbers (should be faster than pow(x, 2.0))
namespace { constexpr double squared(double x) { return x * x; } }

/**
 * Literature referenced here:
 * Rappaport, Theodore: Wireless Communications - Principles and Practice, 2nd edition, 2002
 * Jakes, William C.: "Microwave Mobile Communications", 1974
 */

TwoRayInterference::TwoRayInterference() :
    epsilon_r(1.0), polarization('h')
{
}

void TwoRayInterference::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        epsilon_r = par("epsilon_r");
        const std::string polarization_str = par("polarization");
        if (polarization_str == "horizontal") {
            polarization = 'h';
        }
        else if (polarization_str == "vertical") {
            polarization = 'v';
        }
        else {
            throw cRuntimeError("Invalid antenna polarization %s", polarization_str.c_str());
        }
    }
}

std::ostream& TwoRayInterference::printToStream(std::ostream& os, int level, int evFlags) const
{
    os << "TwoRayInterference";
    if (level >= PRINT_LEVEL_TRACE)
        os << ", epsilon_r = " << epsilon_r << EV_FIELD(polarization);
    return os;
}

double TwoRayInterference::computePathLoss(const ITransmission *transmission, const IArrival *arrival) const
{
    auto radioMedium = transmission->getMedium();
    auto narrowbandSignalAnalogModel = check_and_cast<const INarrowbandSignal *>(transmission->getAnalogModel());
    mps propagationSpeed = radioMedium->getPropagation()->getPropagationSpeed();
    Hz centerFrequency = Hz(narrowbandSignalAnalogModel->getCenterFrequency());
    const m waveLength = propagationSpeed / centerFrequency;

    return computeTwoRayInterference(transmission->getStartPosition(), arrival->getStartPosition(), waveLength);
}

double TwoRayInterference::computeTwoRayInterference(const Coord& pos_t, const Coord& pos_r, m lambda) const
{
    const double h_sum = pos_t.z + pos_r.z;
    const double h_diff = pos_t.z - pos_r.z;

    // direct line of sight between Tx and Rx antenna
    const double d_los = pos_r.distance(pos_t);
    // distance on flat ground between Tx and Rx
    const double d_grnd = sqrt(squared(d_los) - squared(h_diff));
    // length of reflection path between antennas
    const double d_refl = sqrt(squared(d_grnd) + squared(h_sum));

    // phi is phase difference of LOS and reflected signal
    const double phi = 2.0 * M_PI * (d_refl - d_los) / lambda.get();
    // theta is the angle between reflected ray and ground (angle of incidence)
    const double sin_theta = h_sum / d_refl;
    const double cos_theta = d_grnd / d_refl;
    // reflection coefficient depending on polarization and relative permittivity (epsilon_r)
    const double Gamma = reflectionCoefficient(cos_theta, sin_theta);

    /*
     * Original equation from Jakes (eq 2.1-2) adapted to our variable names:
     *
     * P_r = P_t * G_t * G_r * (lambda / (4*pi*d_los))^2 * |1 + Gamma * e^(i phi)|^2
     *
     * Notes:
     * - return value by IPathLoss equals P_r / (P_t * G_t * G_r)
     * - (lambda / (4*pi*d_los))^2 is the free space path loss (part of Friis equation)
     * - squared absolute value term could be named "interference term"
     *   - "1" accounts for direct wave (LOS)
     *   - "Gamma * e^(i phi)" accounts for ground reflected wave
     *
     * |1 + Gamma * e^(i phi)|^2 = (applying Euler's formula)
     * (1 + Gamma * cos(phi))^2 - Gamma^2 * sin(phi)^2
     */

    const double friis_term = squared(lambda.get() / (4.0 * M_PI * d_los));
    const double interference_term = squared(1.0 + Gamma * cos(phi)) + squared(Gamma * sin(phi));
    return friis_term * interference_term;
}

double TwoRayInterference::computePathLoss(mps propagationSpeed, Hz frequency, m distance) const
{
    return NaN;
}

m TwoRayInterference::computeRange(mps propagationSpeed, Hz frequency, double loss) const
{
    return m(NaN);
}

double TwoRayInterference::reflectionCoefficient(double cos_theta, double sin_theta) const
{
    double Gamma = 0.0;
    // sqrt_term is named "z" by Jakes (eq 2.1-3)
    const double sqrt_term = sqrt(epsilon_r - squared(cos_theta));
    switch (polarization) {
        case 'h':
            // equation 4.25 in Rappaport (Gamma orthogonal)
            Gamma = (sin_theta - sqrt_term) / (sin_theta + sqrt_term);
            break;
        case 'v':
            // equation 4.24 in Rappaport (Gamma parallel)
            Gamma = (-epsilon_r * sin_theta + sqrt_term) / (epsilon_r * sin_theta + sqrt_term);
            break;
        default:
            throw cRuntimeError("Unknown polarization");
    }
    return Gamma;
}

} // namespace physicallayer
} // namespace artery

