//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.mobility.single;

import inet.mobility.base.MobilityBase;
import inet.mobility.contract.IMobility;

//
// Re-bases a contained ("local") mobility model onto a geographic anchor in the
// global Earth frame. The child `mobility` submodule produces positions in a
// local East-North-Up frame; this module pins that frame to the geographic point
// given by `anchorLatitude`, `anchorLongitude` and `anchorAltitude`, optionally
// rotates the local frame with `anchorHeading`/`anchorElevation`/`anchorBank` so
// the local axes can be aligned in any direction (e.g. a road running NE, a runway
// heading), and reports the resulting position through the geographic coordinate
// system referenced by `coordinateSystemModule`.
//
// This lets any existing terrestrial mobility model (car, UAV, ship) drive a
// geographically-anchored, arbitrarily-oriented node in a global scene (typically
// a ~GeocentricCoordinateSystem) shared with satellites and other ground equipment.
//
module GeographicAnchorMobility extends MobilityBase
{
    parameters:
        double anchorLatitude @unit(deg); // Latitude of the anchor the local frame is pinned to
        double anchorLongitude @unit(deg); // Longitude of the anchor the local frame is pinned to
        double anchorAltitude @unit(m) = default(0m); // Ellipsoidal height of the anchor
        double anchorHeading @unit(deg) = default(0deg); // Rotation of the local frame about the local up axis (positive turns left)
        double anchorElevation @unit(deg) = default(0deg); // Rotation of the local frame about the local east axis (positive lifts north up)
        double anchorBank @unit(deg) = default(0deg); // Rotation of the local frame about the local north axis
        mobility.subjectModule = default(""); // The local mobility must not drive the subject module's display directly
        @class(GeographicAnchorMobility);
    submodules:
        mobility: <default("StationaryMobility")> like IMobility {
            parameters:
                @display("p=100,100");
        }
}

