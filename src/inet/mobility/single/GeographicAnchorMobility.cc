//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/mobility/single/GeographicAnchorMobility.h"

#include "inet/common/geometry/common/Wgs84.h"

namespace inet {

Define_Module(GeographicAnchorMobility);

void GeographicAnchorMobility::initialize(int stage)
{
    MobilityBase::initialize(stage);
    EV_TRACE << "initializing GeographicAnchorMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        coordinateSystem = findModuleFromPar<IGeographicCoordinateSystem>(par("coordinateSystemModule"), this);
        GeoCoord anchor(deg(par("anchorLatitude")), deg(par("anchorLongitude")), m(par("anchorAltitude")));
        anchorEcef = wgs84::geodeticToEcef(anchor);
        // local orientation offset within the ENU frame (negation of elevation as in IMobility)
        auto alpha = deg(par("anchorHeading"));
        auto beta = -deg(par("anchorElevation"));
        auto gamma = deg(par("anchorBank"));
        Quaternion orientationOffset(EulerAngles(alpha, beta, gamma));
        // local -> ECEF rotation: first apply the local offset, then ENU -> ECEF at the anchor
        combinedRotation = wgs84::enuToEcefRotation(anchor) * orientationOffset;
        auto element = getSubmodule("mobility");
        if (element == nullptr)
            throw cRuntimeError("GeographicAnchorMobility requires a 'mobility' submodule");
        element->subscribe(IMobility::mobilityStateChangedSignal, this);
        mobility = check_and_cast<IMobility *>(element);
        WATCH(anchorEcef);
        WATCH(combinedRotation);
        WATCH(lastVelocity);
    }
    else if (stage == INITSTAGE_LAST)
        initializePosition();
}

void GeographicAnchorMobility::setInitialPosition()
{
    lastPosition = getCurrentPosition();
}

Coord GeographicAnchorMobility::ecefToScene(const Coord& ecef) const
{
    if (coordinateSystem != nullptr)
        return coordinateSystem->computeSceneCoordinate(wgs84::ecefToGeodetic(ecef));
    else
        // no coordinate system: the scene frame is assumed to coincide with ECEF
        return ecef;
}

void GeographicAnchorMobility::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));
    if (IMobility::mobilityStateChangedSignal == signal)
        emitMobilityStateChangedSignal();
}

const Coord& GeographicAnchorMobility::getCurrentPosition()
{
    Coord ecef = anchorEcef + combinedRotation.rotate(mobility->getCurrentPosition());
    lastPosition = ecefToScene(ecef);
    return lastPosition;
}

const Coord& GeographicAnchorMobility::getCurrentVelocity()
{
    lastVelocity = combinedRotation.rotate(mobility->getCurrentVelocity());
    return lastVelocity;
}

const Coord& GeographicAnchorMobility::getCurrentAcceleration()
{
    lastAcceleration = combinedRotation.rotate(mobility->getCurrentAcceleration());
    return lastAcceleration;
}

const Quaternion& GeographicAnchorMobility::getCurrentAngularPosition()
{
    lastOrientation = combinedRotation * mobility->getCurrentAngularPosition();
    return lastOrientation;
}

const Quaternion& GeographicAnchorMobility::getCurrentAngularVelocity()
{
    lastAngularVelocity = mobility->getCurrentAngularVelocity();
    return lastAngularVelocity;
}

} // namespace inet

