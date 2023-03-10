//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

#include "inet/mobility/base/MobilityBase.h"

#include "inet/common/INETMath.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/common/geometry/common/Quaternion.h"

#ifdef INET_WITH_VISUALIZATIONCANVAS
#include "inet/visualizer/canvas/mobility/MobilityCanvasVisualizer.h"
#endif

namespace inet {

Register_Abstract_Class(MobilityBase);

static bool parseIntTo(const char *s, double& destValue)
{
    if (!s || !*s)
        return false;

    /* This method is only used to convert positions from the display strings,
     * which can contain floating point values.
     */
    if (sscanf(s, "%lf", &destValue) != 1)
        return false;

    return true;
}

static bool isFiniteNumber(double value)
{
    return value <= DBL_MAX && value >= -DBL_MAX;
}

MobilityBase::MobilityBase() :
    subjectModule(nullptr),
    canvasProjection(nullptr),
    constraintAreaMin(Coord::ZERO),
    constraintAreaMax(Coord::ZERO),
    lastPosition(Coord::ZERO)
{
}

std::string MobilityBase::DirectiveResolver::resolveDirective(char directive) const
{
    switch (directive) {
        case 'p':
            return mobility->getCurrentPosition().str();
        case 'v':
            return mobility->getCurrentVelocity().str();
        case 's':
            return std::to_string(mobility->getCurrentVelocity().length());
        case 'a':
            return mobility->getCurrentAcceleration().str();
        case 'P':
            return mobility->getCurrentAngularPosition().str();
        case 'V':
            return mobility->getCurrentAngularVelocity().str();
        case 'S': {
            auto angularVelocity = mobility->getCurrentAngularVelocity();
            Coord axis;
            double angle;
            angularVelocity.getRotationAxisAndAngle(axis, angle);
            return std::to_string(angle);
        }
        case 'A':
            return mobility->getCurrentAngularAcceleration().str();
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
}

void MobilityBase::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    EV_TRACE << "initializing MobilityBase stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        constraintAreaMin.x = par("constraintAreaMinX");
        constraintAreaMin.y = par("constraintAreaMinY");
        constraintAreaMin.z = par("constraintAreaMinZ");
        constraintAreaMax.x = par("constraintAreaMaxX");
        constraintAreaMax.y = par("constraintAreaMaxY");
        constraintAreaMax.z = par("constraintAreaMaxZ");
        format.parseFormat(par("displayStringTextFormat"));
        subjectModule = findSubjectModule();
        if (subjectModule != nullptr) {
            auto visualizationTarget = subjectModule->getParentModule();
            canvasProjection = CanvasProjection::getCanvasProjection(visualizationTarget->getCanvas());
        }
        WATCH(constraintAreaMin);
        WATCH(constraintAreaMax);
        WATCH(lastPosition);
        WATCH(lastOrientation);
    }
    else if (stage == INITSTAGE_SINGLE_MOBILITY) {
        initializeOrientation();
        initializePosition();
    }
}

void MobilityBase::initializePosition()
{
    setInitialPosition();
    checkPosition();
    emitMobilityStateChangedSignal();
}

void MobilityBase::setInitialPosition()
{
    // reading the coordinates from omnetpp.ini makes predefined scenarios a lot easier
    auto coordinateSystem = findModuleFromPar<IGeographicCoordinateSystem>(par("coordinateSystemModule"), this);
    if (subjectModule != nullptr && hasPar("initFromDisplayString") && par("initFromDisplayString")) {
        const char *s = subjectModule->getDisplayString().getTagArg("p", 2);
        if (s && *s)
            throw cRuntimeError("The coordinates of '%s' are invalid. Please remove automatic arrangement"
                                " (3rd argument of 'p' tag) from '@display' attribute.", subjectModule->getFullPath().c_str());
        bool filled = parseIntTo(subjectModule->getDisplayString().getTagArg("p", 0), lastPosition.x) &&
                      parseIntTo(subjectModule->getDisplayString().getTagArg("p", 1), lastPosition.y);
        if (filled) {
            lastPosition.z = hasPar("initialZ") ? par("initialZ") : 0.0;
            lastPosition = canvasProjection->computeCanvasPointInverse(cFigure::Point(lastPosition.x, lastPosition.y), lastPosition.z);
            EV_DEBUG << "position initialized from displayString and initialZ parameter: " << lastPosition << endl;
        }
        else {
            lastPosition = getRandomPosition();
            EV_DEBUG << "position initialized by random values: " << lastPosition << endl;
        }
    }
    // not all mobility models have "initialX", "initialY" and "initialZ" parameters
    else if (coordinateSystem == nullptr && hasPar("initialX") && hasPar("initialY") && hasPar("initialZ")) {
        lastPosition.x = par("initialX");
        lastPosition.y = par("initialY");
        lastPosition.z = par("initialZ");
        EV_DEBUG << "position initialized from initialX/Y/Z parameters: " << lastPosition << endl;
    }
    else if (coordinateSystem != nullptr && hasPar("initialLatitude") && hasPar("initialLongitude") && hasPar("initialAltitude")) {
        auto initialLatitude = deg(par("initialLatitude"));
        auto initialLongitude = deg(par("initialLongitude"));
        auto initialAltitude = m(par("initialAltitude"));
        lastPosition = coordinateSystem->computeSceneCoordinate(GeoCoord(initialLatitude, initialLongitude, initialAltitude));
        EV_DEBUG << "position initialized from initialLatitude/Longitude/Altitude parameters: " << lastPosition << endl;
    }
    else {
        lastPosition = getRandomPosition();
        EV_DEBUG << "position initialized by random values: " << lastPosition << endl;
    }
    if (par("updateDisplayString"))
        updateDisplayStringFromMobilityState();
}

void MobilityBase::checkPosition()
{
    if (!isFiniteNumber(lastPosition.x) || !isFiniteNumber(lastPosition.y) || !isFiniteNumber(lastPosition.z))
        throw cRuntimeError("Mobility position is not a finite number after initialize (x=%g,y=%g,z=%g)", lastPosition.x, lastPosition.y, lastPosition.z);
    if (isOutside())
        throw cRuntimeError("Mobility position (x=%g,y=%g,z=%g) is outside the constraint area (%g,%g,%g - %g,%g,%g)",
                lastPosition.x, lastPosition.y, lastPosition.z,
                constraintAreaMin.x, constraintAreaMin.y, constraintAreaMin.z,
                constraintAreaMax.x, constraintAreaMax.y, constraintAreaMax.z);
}

void MobilityBase::initializeOrientation()
{
    if (hasPar("initialHeading") && hasPar("initialElevation") && hasPar("initialBank")) {
        auto alpha = deg(par("initialHeading"));
        auto initialElevation = deg(par("initialElevation"));
        // NOTE: negation is needed, see IMobility comments on orientation
        auto beta = -initialElevation;
        auto gamma = deg(par("initialBank"));
        lastOrientation = Quaternion(EulerAngles(alpha, beta, gamma));
    }
}

void MobilityBase::refreshDisplay() const
{
    DirectiveResolver directiveResolver(const_cast<MobilityBase *>(this));
    auto text = format.formatString(&directiveResolver);
    getDisplayString().setTagArg("t", 0, text.c_str());
    if (par("updateDisplayString"))
        updateDisplayStringFromMobilityState();
}

void MobilityBase::updateDisplayStringFromMobilityState() const
{
    if (subjectModule != nullptr) {
        // position
        auto position = const_cast<MobilityBase *>(this)->getCurrentPosition();
        EV_TRACE << "current position = " << position << endl;
        auto subjectModulePosition = canvasProjection->computeCanvasPoint(position);
        char buf[32];
        snprintf(buf, sizeof(buf), "%lf", subjectModulePosition.x);
        buf[sizeof(buf) - 1] = 0;
        auto& displayString = subjectModule->getDisplayString();
        displayString.setTagArg("p", 0, buf);
        snprintf(buf, sizeof(buf), "%lf", subjectModulePosition.y);
        buf[sizeof(buf) - 1] = 0;
        displayString.setTagArg("p", 1, buf);
        // angle
        double angle = 0;
        auto angularPosition = const_cast<MobilityBase *>(this)->getCurrentAngularPosition();
        if (angularPosition != Quaternion::IDENTITY) {
            Quaternion swing;
            Quaternion twist;
            Coord vector = canvasProjection->computeCanvasPointInverse(cFigure::Point(0, 0), 1);
            vector.normalize();
            angularPosition.getSwingAndTwist(vector, swing, twist);
            Coord axis;
            twist.getRotationAxisAndAngle(axis, angle);
            angle = math::rad2deg(axis.z >= 0 ? angle : -angle);
        }
        snprintf(buf, sizeof(buf), "%lf", angle);
        displayString.setTagArg("a", 0, buf);
    }
}

void MobilityBase::handleParameterChange(const char *name)
{
    if (!strcmp(name, "displayStringTextFormat"))
        format.parseFormat(par("displayStringTextFormat"));
}

void MobilityBase::handleMessage(cMessage *message)
{
    if (message->isSelfMessage())
        handleSelfMessage(message);
    else
        throw cRuntimeError("Mobility modules can only receive self messages");
}

void MobilityBase::emitMobilityStateChangedSignal()
{
    emit(mobilityStateChangedSignal, this);
}

Coord MobilityBase::getRandomPosition()
{
    Coord p;
    p.x = uniform(constraintAreaMin.x, constraintAreaMax.x);
    p.y = uniform(constraintAreaMin.y, constraintAreaMax.y);
    p.z = uniform(constraintAreaMin.z, constraintAreaMax.z);
    return p;
}

bool MobilityBase::isOutside()
{
    return lastPosition.x < constraintAreaMin.x || lastPosition.x > constraintAreaMax.x
           || lastPosition.y < constraintAreaMin.y || lastPosition.y > constraintAreaMax.y
           || lastPosition.z < constraintAreaMin.z || lastPosition.z > constraintAreaMax.z;
}

static int reflect(double min, double max, double& coordinate, double& speed)
{
    double size = max - min;
    double value = coordinate - min;
    int sign = 1 - math::modulo(floor(value / size), 2) * 2;
    ASSERT(sign == 1 || sign == -1);
    coordinate = math::modulo(sign * value, size) + min;
    speed = sign * speed;
    return sign;
}

void MobilityBase::reflectIfOutside(Coord& targetPosition, Coord& velocity, rad& heading, rad& elevation, Quaternion& quaternion)
{
    int sign;
    double dummy = NaN;
    if (lastPosition.x < constraintAreaMin.x || constraintAreaMax.x < lastPosition.x) {
        sign = reflect(constraintAreaMin.x, constraintAreaMax.x, lastPosition.x, velocity.x);
        reflect(constraintAreaMin.x, constraintAreaMax.x, targetPosition.x, dummy);
        heading = deg(90) + (heading - deg(90)) * sign;
        if (sign == -1 && quaternion != Quaternion::NIL) {
            std::swap(quaternion.s, quaternion.v.z);
            std::swap(quaternion.v.x, quaternion.v.y);
            quaternion.v.x *= -1;
            quaternion.v.y *= -1;
        }
    }
    if (lastPosition.y < constraintAreaMin.y || constraintAreaMax.y < lastPosition.y) {
        sign = reflect(constraintAreaMin.y, constraintAreaMax.y, lastPosition.y, velocity.y);
        reflect(constraintAreaMin.y, constraintAreaMax.y, targetPosition.y, dummy);
        heading = heading * sign;
        if (sign == -1 && quaternion != Quaternion::NIL) {
            quaternion.v.x *= -1;
            quaternion.v.z *= -1;
        }
    }
    if (lastPosition.z < constraintAreaMin.z || constraintAreaMax.z < lastPosition.z) {
        sign = reflect(constraintAreaMin.z, constraintAreaMax.z, lastPosition.z, velocity.z);
        reflect(constraintAreaMin.z, constraintAreaMax.z, targetPosition.z, dummy);
        elevation = elevation * sign;
        if (sign == -1 && quaternion != Quaternion::NIL) {
            quaternion.v.x *= -1;
            quaternion.v.y *= -1;
        }
    }
}

static void wrap(double min, double max, double& coordinate)
{
    coordinate = math::modulo(coordinate - min, max - min) + min;
}

void MobilityBase::wrapIfOutside(Coord& targetPosition)
{
    if (lastPosition.x < constraintAreaMin.x || constraintAreaMax.x < lastPosition.x) {
        wrap(constraintAreaMin.x, constraintAreaMax.x, lastPosition.x);
        wrap(constraintAreaMin.x, constraintAreaMax.x, targetPosition.x);
    }
    if (lastPosition.y < constraintAreaMin.y || constraintAreaMax.y < lastPosition.y) {
        wrap(constraintAreaMin.y, constraintAreaMax.y, lastPosition.y);
        wrap(constraintAreaMin.y, constraintAreaMax.y, targetPosition.y);
    }
    if (lastPosition.z < constraintAreaMin.z || constraintAreaMax.z < lastPosition.z) {
        wrap(constraintAreaMin.z, constraintAreaMax.z, lastPosition.z);
        wrap(constraintAreaMin.z, constraintAreaMax.z, targetPosition.z);
    }
}

void MobilityBase::placeRandomlyIfOutside(Coord& targetPosition)
{
    if (isOutside()) {
        Coord newPosition = getRandomPosition();
        targetPosition += newPosition - lastPosition;
        lastPosition = newPosition;
    }
}

void MobilityBase::raiseErrorIfOutside()
{
    if (isOutside()) {
        throw cRuntimeError("Mobility moved outside the area %g,%g,%g - %g,%g,%g (x=%g,y=%g,z=%g)",
                constraintAreaMin.x, constraintAreaMin.y, constraintAreaMin.z,
                constraintAreaMax.x, constraintAreaMax.y, constraintAreaMax.z,
                lastPosition.x, lastPosition.y, lastPosition.z);
    }
}

void MobilityBase::handleIfOutside(BorderPolicy policy, Coord& targetPosition, Coord& velocity)
{
    rad a;
    Quaternion nil = Quaternion::NIL;
    handleIfOutside(policy, targetPosition, velocity, a, a, nil);
}

void MobilityBase::handleIfOutside(BorderPolicy policy, Coord& targetPosition, Coord& velocity, rad& heading)
{
    rad dummy;
    Quaternion nil = Quaternion::NIL;
    handleIfOutside(policy, targetPosition, velocity, heading, dummy, nil);
}

void MobilityBase::handleIfOutside(BorderPolicy policy, Coord& targetPosition, Coord& velocity, rad& heading, rad& elevation)
{
    Quaternion nil = Quaternion::NIL;
    handleIfOutside(policy, targetPosition, velocity, heading, elevation, nil);
}

void MobilityBase::handleIfOutside(BorderPolicy policy, Coord& targetPosition, Coord& velocity, rad& heading, rad& elevation, Quaternion& quaternion)
{
    switch (policy) {
        case REFLECT:
            reflectIfOutside(targetPosition, velocity, heading, elevation, quaternion);
            break;

        case WRAP:
            wrapIfOutside(targetPosition);
            break;

        case PLACERANDOMLY:
            placeRandomlyIfOutside(targetPosition);
            break;

        case RAISEERROR:
            raiseErrorIfOutside();
            break;

        default:
            throw cRuntimeError("Invalid outside policy=%d in module '%s'", policy, getFullPath().c_str());
    }
}

} // namespace inet

