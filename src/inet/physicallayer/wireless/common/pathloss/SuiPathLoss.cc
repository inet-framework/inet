//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
/***************************************************************************
* author:      Konrad Polys, Krzysztof Grochla
*
* copyright:   (c) 2013 The Institute of Theoretical and Applied Informatics
*                       of the Polish Academy of Sciences, Project
*                       LIDER/10/194/L-3/11/ supported by NCBIR
*
***************************************************************************/

#include "inet/physicallayer/wireless/common/pathloss/SuiPathLoss.h"

namespace inet {

namespace physicallayer {

Define_Module(SuiPathLoss);

SuiPathLoss::SuiPathLoss() :
    ht(m(0)),
    hr(m(0)),
    a(0),
    b(0),
    c(0),
    d(0),
    s(0)
{
}

void SuiPathLoss::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        ht = m(par("transmitterAntennaHeight"));
        hr = m(par("receiverAntennaHeight"));
        /**
         * Terrain A - Highest path loss. Dense populated urban area.
         * Terrain B - Intermediate path loss. Suburban area.
         * Terrain C - Minimum path loss. Flat areas or rural with light vegetation.
         */
        const char *terrain = par("terrain");
        if (!strcmp(terrain, "TerrainA")) {
            a = 4.6;
            b = 0.0075;
            c = 12.6;
            d = 10.8;
            s = 10.6;
        }
        else if (!strcmp(terrain, "TerrainB")) {
            a = 4.0;
            b = 0.0065;
            c = 17.1;
            d = 10.8;
            s = 9.6;
        }
        else if (!strcmp(terrain, "TerrainC")) {
            a = 3.6;
            b = 0.0050;
            c = 20.0;
            d = 20.0;
            s = 8.2;
        }
        else
            throw cRuntimeError("Unknown terrain");
    }
}

std::ostream& SuiPathLoss::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "SuiPathLoss";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(ht)
               << EV_FIELD(hr);
    return stream;
}

double SuiPathLoss::computePathLoss(mps propagationSpeed, Hz frequency, m distance) const
{
    m R = distance;
    m R0 = m(100.0);
    m lambda = propagationSpeed / frequency;
    double L = 0.0; // [dBm]
    double f = frequency.get() / 1000000000.0; // [GHz]
    double alpha = 0.0;
    double gamma = a - b * ht.get() + c / ht.get();
    double Xf = 6 * log10(f / 2);
    double Xh = -d *log10(hr.get() / 2);
    m R0p = R0 * pow(10.0, -((Xf + Xh) / (10 * gamma)));
    if (R > R0p) {
        alpha = 20 * log10(unit(4 * M_PI * R0p / lambda).get());
        L = alpha + 10 * gamma * log10(unit(R / R0).get()) + Xf + Xh + s;
    }
    else {
        L = 20 * log10(unit(4 * M_PI * R / lambda).get()) + s;
    }
    return math::dB2fraction(-L);
}

} // namespace physicallayer

} // namespace inet

