//
// ObstacleControl - models obstacles that block radio transmissions
// Copyright (C) 2010 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

package inet.world.obstacles;

//
// ObstacleControl models obstacles that block radio transmissions
//
// The XML:
//   <obstacles>
//     <poly
//       id="building#0"
//       type="building"
//       color="#F00"
//       shape="16,0 8,13.8564 -8,13.8564 -16,0 -8,-13.8564 8,-13.8564"
//       attenuationPerWall="50dB"
//       attenuationPerMeter="1dB"
//     />
//       ...
//   </obstacles>
//
//  The attributes of 'poly' element:
//    - id: identifier, required.
//    - type: Type of obstacle, must be "building", required.
//    - color: Color of obstacle, required, used by AnnotationManager.
//    - shape: Shape of polygon obstacle, required, the walls of obstacle.
//             The last and first point will be connected, also.
//    - attenuationPerWall: Attenaution when wave crossed the wall [dB].
//    - attenuationPerMeter: Attenaution in inside of obstacle [dB/m].
//
simple ObstacleControl
{
    parameters:
        bool debug = default(false);  // emit debug messages?
        xml obstacles = default(xml("<obstacles/>")); // obstacles to add at startup
        double attenuationPerWall @unit(dB) = default(50dB); //default attenaution per wall
        double attenuationPerMeter @unit(dB) = default(1dB); //default attenaution in inside of obstacle in dB/m
        @display("i=misc/town");
        @labels(node);
}

