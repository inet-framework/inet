//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.environment.contract;

//
// Module interface for physical environment models that contain physical objects
// affecting signal propagation. Implementations define a space with objects of
// various shapes, positions, orientations, and materials. These objects impact
// wireless signal propagation through absorption, reflection, refraction, and
// diffraction.
//
moduleinterface IPhysicalEnvironment
{
    parameters:
        @display("i=misc/town");
}
