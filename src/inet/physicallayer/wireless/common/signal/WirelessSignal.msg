//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

cplusplus {{
#include "inet/physicallayer/wireless/common/signal/WirelessSignal.h"
}}

import inet.physicallayer.common.Signal;

namespace inet::physicallayer;

class ITransmission { @existingClass; }
class IArrival { @existingClass; }
class IListening { @existingClass; }
class IReception { @existingClass; }

packet WirelessSignal extends Signal
{
    @existingClass;
    @descriptor(readonly);
    const ITransmission *transmission;
    const IArrival *arrival;
    const IListening *listening;
    const IReception *reception;
}

