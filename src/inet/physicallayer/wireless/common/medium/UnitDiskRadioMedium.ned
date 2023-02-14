//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.medium;

//
// This radio medium model provides a very simple but fast and predictable
// physical layer behavior. It must be used in conjunction with the ~UnitDiskRadio
// model.
//
// @see ~UnitDiskRadio, ~UnitDiskMediumAnalogModel, ~UnitDiskTransmitter,
// ~UnitDiskReceiver.
//
module UnitDiskRadioMedium extends RadioMedium
{
    parameters:
        signalAnalogRepresentation = default("unitDisk");
}

