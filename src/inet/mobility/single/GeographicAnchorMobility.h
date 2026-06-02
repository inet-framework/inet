//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GEOGRAPHICANCHORMOBILITY_H
#define __INET_GEOGRAPHICANCHORMOBILITY_H

#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/mobility/base/MobilityBase.h"

namespace inet {

/**
 * Re-bases a contained ("local") mobility model onto a geographic anchor in the
 * global Earth frame. The child mobility produces positions in a local
 * East-North-Up frame; this module pins that frame to a geographic point
 * (anchorLatitude/Longitude/Altitude), optionally rotates it
 * (anchorHeading/Elevation/Bank) so the local axes can be aligned in any
 * direction, and reports the result through the geographic coordinate system.
 *
 * This lets any existing terrestrial mobility model (e.g. a car, UAV or ship
 * moving in local coordinates) drive a geographically-anchored node in a global
 * (e.g. geocentric ECEF) scene shared with satellites and other ground equipment.
 *
 * The transform is: scene = coordinateSystem.computeSceneCoordinate(
 *     ecefToGeodetic( anchorEcef + R_enu(anchor) * R_orient * p_local ) ).
 * Because the anchor is fixed, the local->ECEF map has a constant linear part, so
 * velocity/orientation are transformed by the same constant rotation (this assumes
 * the scene frame is ECEF, i.e. a ~GeocentricCoordinateSystem is used).
 */
class INET_API GeographicAnchorMobility : public MobilityBase, public cListener
{
  protected:
    const IGeographicCoordinateSystem *coordinateSystem = nullptr;
    IMobility *mobility = nullptr;

    /** Precomputed anchor position in ECEF and the local->ECEF rotation. */
    Coord anchorEcef = Coord::ZERO;
    Quaternion combinedRotation = Quaternion::IDENTITY;

    Coord lastVelocity = Coord::ZERO;
    Coord lastAcceleration = Coord::ZERO;
    Quaternion lastAngularVelocity = Quaternion::IDENTITY;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void setInitialPosition() override;
    virtual void handleSelfMessage(cMessage *msg) override { throw cRuntimeError("Unknown self message"); }

    /** Maps an ECEF position into the scene coordinate frame. */
    virtual Coord ecefToScene(const Coord& ecef) const;

  public:
    virtual const Coord& getCurrentPosition() override;
    virtual const Coord& getCurrentVelocity() override;
    virtual const Coord& getCurrentAcceleration() override;

    virtual const Quaternion& getCurrentAngularPosition() override;
    virtual const Quaternion& getCurrentAngularVelocity() override;
    virtual const Quaternion& getCurrentAngularAcceleration() override { return Quaternion::IDENTITY; }

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace inet

#endif

