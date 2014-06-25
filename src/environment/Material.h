//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_MATERIAL_H_
#define __INET_MATERIAL_H_

#include <limits>
#include "INETDefs.h"
#include "Units.h"

using namespace units::values;
using namespace units::constants;

// TODO: merge
#define sNaN std::numeric_limits<double>::signaling_NaN()

class INET_API Material
{
    protected:
        Ohmm resistivity;
        double relativePermittivity;
        double relativePermeability;

//        double reflectionLossPerWall;
//        double dielectricLossPerMeter;
//        double attenuationPerWall; /**< in dB. Consumer Wi-Fi vs. an exterior wall will give approx. 50 dB */
//        double attenuationPerMeter; /**< in dB / m. Consumer Wi-Fi vs. an interior hollow wall will give approx. 5 dB */

    public:
        static Material wood;
        static Material concrete;
        static Material glass;

    public:
        Material(Ohmm resistivity, double relativePermittivity, double relativePermeability);

        virtual Ohmm getResistivity() const { return resistivity; }
        virtual double getRelativePermittivity() const { return relativePermittivity; }
        virtual double getRelativePermeability() const { return relativePermeability; }
        virtual double getDielectricLossTangent(Hz frequency) const;
        virtual double getRefractiveIndex() const;
        virtual mps getPropagationSpeed() const;
};

#endif
