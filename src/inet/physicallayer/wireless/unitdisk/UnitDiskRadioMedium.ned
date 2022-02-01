//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.unitdisk;

import inet.physicallayer.wireless.common.medium.RadioMedium;

//
// This radio medium model provides a very simple but fast and predictable
// physical layer behavior. It must be used in conjunction with the ~UnitDiskRadio
// model.
//
// @see ~UnitDiskRadio, ~UnitDiskAnalogModel, ~UnitDiskTransmitter,
// ~UnitDiskReceiver.
//
module UnitDiskRadioMedium extends RadioMedium
{
    parameters:
        analogModel.typename = default("UnitDiskAnalogModel");
        signalAnalogRepresentation = default("unitDisk");
}

