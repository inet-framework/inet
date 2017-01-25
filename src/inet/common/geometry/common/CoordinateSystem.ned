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

package inet.common.geometry.common;

//
// This module interface is used by geogrpahic coordinate systems. A geographic
// coordinate system maps playground coordinates to geographic coordinates, and
// vice versa.
//
moduleinterface IGeographicCoordinateSystem
{
    parameters:
        @display("i=block/table2_s");
}

//
// This module provides a very simple and less accurate geographic coordinate
// system without using OSG. It doesn't support orientation
//
simple SimpleGeographicCoordinateSystem like IGeographicCoordinateSystem
{
    parameters:
        @display("i=block/table2_s");
        double playgroundLatitude @unit(deg); // latitude coordinate of the playground origin
        double playgroundLongitude @unit(deg); // longitude coordinate of the playground origin
        double playgroundAltitude @unit(m) = default(0m); // altitude coordinate of the playground origin
}

//
// This module provides an accurate geographic coordinate system using the
// built-in OSG API.
//
simple OsgGeographicCoordinateSystem like IGeographicCoordinateSystem
{
    parameters:
        @display("i=block/table2_s");
        double playgroundLatitude @unit(deg); // latitude coordinate of the playground origin
        double playgroundLongitude @unit(deg); // longitude coordinate of the playground origin
        double playgroundAltitude @unit(m) = default(0m); // altitude coordinate of the playground origin
        double playgroundHeading @unit(deg) = default(0deg); // heading of the playground
        double playgroundElevation @unit(deg) = default(0deg); // elevation of the playground
        double playgroundBank @unit(deg) = default(0deg); // bank of the playground
}
