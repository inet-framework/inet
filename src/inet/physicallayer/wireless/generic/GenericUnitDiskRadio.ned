//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.generic;

//
// Radio model that uses a simplified unit disk-based representation for wireless
// transmissions. Determines reception success based solely on distance between
// transmitter and receiver using three key thresholds: communication range,
// interference range, and detection range. Provides a computationally efficient
// alternative to more detailed radio models when precise physical layer behavior
// is not critical, making it suitable for large-scale network simulations.
//
module GenericUnitDiskRadio extends GenericRadio
{
    parameters:
        signalAnalogRepresentation = default("unitDisk");
}
