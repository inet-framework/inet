//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.mobility.single;

import inet.mobility.base.MovingMobilityBase;

//
// A LOGO-style movement model, with the script coming from XML. It can be
// useful for describing random as well as deterministic scenarios.
//
// First, a few examples.
//
// Node will move along a square's borders:
//
// <pre><nohtml>
//     <movement>
//         <set speed="10" angle="180"/>
//         <repeat n="4">
//             <forward d="50"/>
//             <turn angle="90"/>
//         </repeat>
//     </movement>
// </nohtml></pre>
//
// ~MassMobility (in which the node is assumed to have a mass, and cannot turn
// abruptly):
//
// <pre><nohtml>
//     <movement>
//         <repeat>
//             <set speed="uniform(10,20)"/>
//             <turn angle="uniform(-30,30)"/>
//             <forward t="uniform(0.1,1)"/>
//         </repeat>
//     </movement>
// </nohtml></pre>
//
// ~RandomWaypointMobility:
//
// <pre><nohtml>
//     <movement>
//         <repeat>
//             <set speed="uniform(20,60)"/>
//             <moveto x="uniform($MINX,$MAXX)" y="uniform($MINY,$MAXY)"/>
//         </repeat>
//     </movement>
// </nohtml></pre>
//
// Commands (XML element names) are: `repeat`, `set`, `forward`, `turn`, `wait`,
// `moveto`, and `moveby`.
//
// Accepted attributes:
//    - `repeat`: `n` (optional)
//    - `set`: `x`, `y`, `speed`, `angle`, `borderPolicy` (all optional)
//    - `forward`: `d`, `t` (at least one of them must be present)
//    - `turn`: `angle` (required)
//    - `wait`: `t` (required)
//    - `moveto`: `x`, `y`, `t` (all optional)
//    - `moveby`: `x`, `y`, `t` (all optional)
//
// Attribute values can be literal numbers (e.g. `x="100"`), or any
// distribution accepted in omnetpp.ini (e.g. `speed="uniform(10,20)"`).
// A new value will be chosen each time the statement is executed.
// The `$MINX`, `$MAXX`, `$MINY` and `$MAXY` strings will be substituted with the
// limits of the constraint area, respectively.
//
// `t` is the time interval, calculated from the end of the previous movement.
// `x`, `y` are absolute coordinates with `<set>` and `<moveto>`, and relative
// with `<moveby>`. `angle` is clockwise in degrees, with 0 pointing west.
// `d` is distance in meters (speed is m/s).
//
// `borderPolicy` controls what happens when a node reaches the edge of the
// constraint area and can take one of the values `reflect`, `wrap`,
// `placerandomly`. In the first case, the node reflects off the wall;
// with `wrap`, it'll come in on the other side (torus); and with
// `placerandomly`, it will be placed at a random position on the
// constraint area.
//
// In addition to the node position, the module maintains two interval variables:
// speed and angle, which can be adjusted by `<set>` and `<turn>`.
// The `<forward>` statement, if only `t` or `d` is given, uses the speed variable.
// If you specify `d` and `t` together, that determines the speed as well,
// so the speed variable will be completely ignored.
//
// Likewise, `<moveto>` and `<moveby>` will ignore the `angle` variable
// and will only use the speed variable if the `t` attribute is omitted.
//
// If the repeat count `n` is omitted for the `<repeat>` statement, that
// means an infinite loop.
//
// You can find the DTD with the sources: TurtleMobility.dtd.
//
simple TurtleMobility extends MovingMobilityBase
{
    parameters:
        xml turtleScript; // Describes the movement
        @class(TurtleMobility);
}

