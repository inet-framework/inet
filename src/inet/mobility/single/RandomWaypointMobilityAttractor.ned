//
// Copyright (C) 2005 Georg Lutz, Institut fuer Telematik, University of Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.mobility.single;

import inet.mobility.base.MovingMobilityBase;

//
// Implements the Random Waypoint with attractos mobility model.
//
// The node moves in line segments. For each line segment, a random destination
// position (distributed uniformly over the constraint area or volume) and a 
// random speed is chosen. You can define a speed as a variate from which a new
// value will be drawn for each line segment; it is customary to specify it as
// uniform(minSpeed, maxSpeed). When the node reaches the target position,
// it waits for the time waitTime which can also be defined as a variate.
// After this time the the algorithm calculates a new random position, etc.
//
//
// @author Alfonso Ariza
//

// <configuration>
//   <common>
//    <attractConf  id = "type_1"  type = "attractor" behaviorType = "nearest">
//     	<Xdistribution> uniform(-20,20) </Xdistribution>
//     	<Ydistribution> uniform(-20,20) </Ydistribution>
//     	<proximity distance="100" repeat = "5" />   -- close attractor
//     	<proximity distance="200" repeat = "3" />   -- second close
//     	<proximity distance="300" repeat = "2" />   -- third close
//    </attractConf>
//    <attractConf  id = "type_2" type = "attractor" behaviorType = "nearest">
//     	<Xdistribution> uniform(-30,30) </Xdistribution>
//     	<Ydistribution> uniform(-30,30) </Ydistribution>
//     	<proximity distance="200" repeat = "4" />   -- close attractor
//     	<proximity distance="300" repeat = "3" />   -- second close
//     	<proximity distance="400" repeat = "2" />   -- third close
//     	<proximity distance= "MAX" repeat = "1" />  -- any point
//    </attractConf>
//    <attractConf  id = "type_3" type = "landscape">
//      <Xdistribution> uniform($MINX,$MAXX) </Xdistribution>
//     	<Ydistribution> uniform($MINY,$MAXY) </Ydistribution>
//     	<Zdistribution> uniform($MINZ,$MAXZ) </Zdistribution>
//    </attractConf>
//   </common>
//   <attractor x="100" y="100" z="0" AttractorId="type_1"/>  -- set the position of the attractor
//   <attractor x="200" y="100" z="0" AttractorId="type_1"/>  -- set the position of the attractor
//   <attractor x="300" y="100" z="0" AttractorId="type_1"/>  -- set the position of the attractor
//   <attractor x="300" y="200" z="0" AttractorId="type_2"/>  -- set the position of the attractor
//   <attractor x="300" y="300" z="0" AttractorId="type_2"/>  -- set the position of the attractor
//   <attractor x="300" y="400" z="0" AttractorId="type_2"/>  -- set the position of the attractor
//   <attractor x="300" y="500" z="0" AttractorId="type_2"/>  -- set the position of the attractor   
//   <freeAttractor x="150" y="150" z="0" Xdistribution = "uniform(-30,30)" Ydistribution = "uniform(-30,30)" repetition = "3"/>  -- set the position of the attractor, this is free list, the nodes that doesn't have a type assigned or the type doesn't exist, will use this
// </configuration>

simple RandomWaypointMobilityAttractor extends MovingMobilityBase
{
    parameters:
        xml xmlConfiguration; // describes the movement
        string attractorId;    
        double initialX @unit(m) = default(uniform(this.constraintAreaMinX, this.constraintAreaMaxX));
        double initialY @unit(m) = default(uniform(this.constraintAreaMinY, this.constraintAreaMaxY));
        double initialZ @unit(m) = default(nanToZero(uniform(this.constraintAreaMinZ, this.constraintAreaMaxZ)));
        bool initFromDisplayString = default(true); // enables one time initialization from the subject module's display string
        volatile double speed @unit(mps) = default(0mps); // use uniform(minSpeed, maxSpeed) or another distribution
        volatile double waitTime @unit(s) = default(0s); // wait time between reaching a target and choosing a new one
        @class(RandomWaypointMobilityAttractor);
}

