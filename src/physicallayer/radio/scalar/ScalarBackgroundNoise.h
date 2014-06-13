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

#ifndef __INET_SCALARBACKGROUNDNOISE_H_
#define __INET_SCALARBACKGROUNDNOISE_H_

#include "IBackgroundNoise.h"

namespace radio
{

class INET_API ScalarBackgroundNoise : public cModule, public IBackgroundNoise
{
    protected:
        W power;

    protected:
        virtual void initialize(int stage);

    public:
        ScalarBackgroundNoise() :
            power(W(sNaN))
        {}

        virtual void printToStream(std::ostream &stream) const { stream << "scalar background noise"; }

        virtual W getPower() const { return power; }

        virtual const INoise *computeNoise(const IListening *listening) const;
};

}

#endif
