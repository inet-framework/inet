//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
