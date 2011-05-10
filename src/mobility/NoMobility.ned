//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.mobility;

//
// A placeholder module used when a node does not need mobility support.
// (i.e. when a StandardHost does not have a wireless card so its
// physical location is irrelevant).
//
module NoMobility like IMobility
{
    double x = default(-1); // start x coordinate (-1 = display string position, or random if it's missing)
    double y = default(-1); // start y coordinate (-1 = display string position, or random if it's missing)
    double constraintAreaWidth @unit("m") = default(600m); // x size of the playground (in meters)
    double constraintAreaHeight @unit("m") = default(400m); // y size of the playground (in meters)
}
