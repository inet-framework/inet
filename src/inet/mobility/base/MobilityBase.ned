//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.mobility.base;

import inet.mobility.contract.IMobility;

//
// Abstract base module for mobility models.
//
// Format directives:
//  - %p position
//  - %v velocity
//  - %s speed
//  - %a acceleration
//  - %P angular position
//  - %V angular velocity
//  - %S angular speed
//  - %A angular acceleration
//
simple MobilityBase like IMobility
{
    parameters:
        @signal[mobilityStateChanged](type=inet::MobilityBase);     // It's work in inet, but not work in extended module in other namespace
        @display("i=block/cogwheel");
        string subjectModule = default("^"); // Module path which determines the subject module, the motion of which this mobility model describes, the default value is the parent module
        string coordinateSystemModule = default(""); // Module path of the geographic coordinate system module
        string displayStringTextFormat @mutable = default("p: %p\nv: %v"); // Format string for the mobility module's display string text
        bool updateDisplayString = default(true); // Enables continuous update of the subject module's position via modifying its display string
        double constraintAreaMinX @unit(m) = default(-inf m); // Min x position of the constraint area, unconstrained by default (negative infinity)
        double constraintAreaMinY @unit(m) = default(-inf m); // Min y position of the constraint area, unconstrained by default (negative infinity)
        double constraintAreaMinZ @unit(m) = default(-inf m); // Min z position of the constraint area, unconstrained by default (negative infinity)
        double constraintAreaMaxX @unit(m) = default(inf m); // Max x position of the constraint area, unconstrained by default (positive infinity)
        double constraintAreaMaxY @unit(m) = default(inf m); // Max y position of the constraint area, unconstrained by default (positive infinity)
        double constraintAreaMaxZ @unit(m) = default(inf m); // Max z position of the constraint area, unconstrained by default (positive infinity)
}

