//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.mobility.base;

import inet.common.SimpleModule;
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
simple MobilityBase extends SimpleModule like IMobility
{
    parameters:
        @class(MobilityBase);
        @signal[mobilityStateChanged](type=inet::MobilityBase);     // It works in inet, but not in the extended module in another namespace
        @display("i=block/cogwheel");
        string subjectModule = default("^"); // The module path that determines the subject module, the motion of which this mobility model describes. The default value is the parent module
        string coordinateSystemModule = default(""); // The module path of the geographic coordinate system module
        displayStringTextFormat = default("p: %p\nv: %v"); // The format string for the mobility module's display string text
        bool updateDisplayString = default(true); // Enables continuous update of the subject module's position by modifying its display string
        double constraintAreaMinX @unit(m) = default(-inf m); // The minimum x position of the constraint area. It is unconstrained by default (negative infinity)
        double constraintAreaMinY @unit(m) = default(-inf m); // The minimum y position of the constraint area. It is unconstrained by default (negative infinity)
        double constraintAreaMinZ @unit(m) = default(-inf m); // The minimum z position of the constraint area. It is unconstrained by default (negative infinity)
        double constraintAreaMaxX @unit(m) = default(inf m); // The maximum x position of the constraint area. It is unconstrained by default (positive infinity)
        double constraintAreaMaxY @unit(m) = default(inf m); // The maximum y position of the constraint area. It is unconstrained by default (positive infinity)
        double constraintAreaMaxZ @unit(m) = default(inf m); // The maximum z position of the constraint area. It is unconstrained by default (positive infinity)
}
