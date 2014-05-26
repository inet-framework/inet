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

#ifndef __INET_IRADIOBACKGROUNDNOISE_H_
#define __INET_IRADIOBACKGROUNDNOISE_H_

#include "IRadioSignalReception.h"
#include "IRadioSignalListening.h"
#include "IRadioSignalNoise.h"

/**
 * This interface models a source which provides background noise over space and time.
 */
class INET_API IRadioBackgroundNoise : public IPrintableObject
{
    public:
        // TODO: merge the two computeNoise functions?
        virtual const IRadioSignalNoise *computeNoise(const IRadioSignalListening *listening) const = 0;
        virtual const IRadioSignalNoise *computeNoise(const IRadioSignalReception *reception) const = 0;
};

#endif
