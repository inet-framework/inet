//
// Copyright (C) 2014 OpenSim Ltd.
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

#ifndef __INET_QAM64MODULATION_H_
#define __INET_QAM64MODULATION_H_

#include "APSKModulationBase.h"

namespace inet {
namespace physicallayer {

class INET_API QAM64Modulation : public APSKModulationBase
{
    public:
        static const QAM64Modulation singleton;

    protected:
        static const Complex encodingTable[64];
        static const double kMOD;

    public:
        QAM64Modulation();
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_QAM64MODULATION_H_ */
