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

#ifndef __INET_PHYSICALLAYERDEFS_H_
#define __INET_PHYSICALLAYERDEFS_H_

#include <limits>
#include "FWMath.h"
#include "Units.h"

#define qNaN std::numeric_limits<double>::quiet_NaN()
#define sNaN std::numeric_limits<double>::signaling_NaN()
#define NaN qNaN
#define isNaN(X) std::isnan(X)
#define POSITIVE_INFINITY std::numeric_limits<double>::infinity()
#define NEGATIVE_INFINITY -std::numeric_limits<double>::infinity()

using namespace units::values;

#endif
