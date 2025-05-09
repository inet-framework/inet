//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.medium;

//
// This radio medium model provides a very simple but fast and predictable
// physical layer behavior. The idea behind this analog model is to describe
// transmissions with a few distance-based parameters: communication range,
// interference range, and detection range. Whether the reception is successful
// or not depends on the distance between the transmitter and the receiver.
//
// @see UnitDiskMediumAnalogModel, UnitDiskTransmitterAnalogModel, UnitDiskReceiverAnalogModel.
// @see DimensionalRadioMedium, ScalarRadioMedium
//
module UnitDiskRadioMedium extends RadioMedium
{
    parameters:
        signalAnalogRepresentation = default("unitDisk");
}

