//
// Copyright (C) 2010 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/ModuleAccess.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/misc/NetAnimTrace.h"
#include "inet/mobility/contract/IMobility.h"

namespace inet {

Define_Module(NetAnimTrace);

simsignal_t NetAnimTrace::messageSentSignal = registerSignal("messageSent");
simsignal_t NetAnimTrace::mobilityStateChangedSignal = registerSignal("mobilityStateChanged");

// TODO: after release of OMNeT++ 4.1 final, update this code to similar class in omnetpp/contrib/util

void NetAnimTrace::initialize()
{
    if (!par("enabled"))
        return;

    const char *filename = par("filename");
    f.open(filename, std::ios::out | std::ios::trunc);
    if (f.fail())
        throw cRuntimeError("Cannot open file \"%s\" for writing", filename);

    dump();

    getSimulation()->getSystemModule()->subscribe(POST_MODEL_CHANGE, this);
    getSimulation()->getSystemModule()->subscribe(messageSentSignal, this);
    getSimulation()->getSystemModule()->subscribe(mobilityStateChangedSignal, this);
}

void NetAnimTrace::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module does not handle messages");
}

void NetAnimTrace::finish()
{
    f.close();
}

void NetAnimTrace::dump()
{
    cModule *parent = getSimulation()->getSystemModule();
    for (cModule::SubmoduleIterator it(parent); !it.end(); it++)
        if ((*it) != this)
            addNode(*it);

    for (cModule::SubmoduleIterator it(parent); !it.end(); it++)
        if ((*it) != this)
            for (cModule::GateIterator ig(*it); !ig.end(); ig++)
                if ((*ig)->getType() == cGate::OUTPUT && (*ig)->getNextGate())
                    addLink(*ig);

}

void NetAnimTrace::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    if (signalID == messageSentSignal && !source->isModule()) {
        // record a "packet sent" line
        cChannel *channel = static_cast<cChannel *>(source);    //FIXME check_and_cast?
        cModule *srcModule = channel->getSourceGate()->getOwnerModule();
        if (isRelevantModule(srcModule)) {
            cModule *destModule = channel->getSourceGate()->getNextGate()->getOwnerModule();
            cITimestampedValue *v = check_and_cast<cITimestampedValue *>(obj);
            if (auto datarateChannel = dynamic_cast<cDatarateChannel *>(channel)) {
                cMessage *msg = check_and_cast<cMessage *>(v->objectValue(signalID));
                simtime_t duration = msg->isPacket() ? static_cast<cPacket *>(msg)->getBitLength() / datarateChannel->getDatarate() : 0.0;
                simtime_t delay = datarateChannel->getDelay();
                simtime_t fbTx = v->getTimestamp(signalID);
                simtime_t lbTx = fbTx + duration;
                simtime_t fbRx = fbTx + delay;
                simtime_t lbRx = lbTx + delay;
                f << fbTx << " P " << srcModule->getId() << " " << destModule->getId() << " " << lbTx << " " << fbRx << " " << lbRx << "\n";
            }
            else if (auto delayChannel = dynamic_cast<cDelayChannel *>(channel)) {
                simtime_t fbTx = v->getTimestamp(signalID);
                simtime_t fbRx = fbTx + delayChannel->getDelay();
                f << fbTx << " P " << srcModule->getId() << " " << destModule->getId() << " " << fbTx << " " << fbRx << " " << fbRx << "\n";
            }
        }
    }
    else if (signalID == mobilityStateChangedSignal) {
        IMobility *mobility = dynamic_cast<IMobility *>(source);
        if (mobility) {
            Coord c = mobility->getCurrentPosition();
            cModule *mod = findContainingNode(dynamic_cast<cModule *>(source));
            if (mod && isRelevantModule(mod))
                f << simTime() << " N " << mod->getId() << " " << c.x << " " << c.y << "\n";
        }
    }
    else if (signalID == POST_MODEL_CHANGE) {
        // record dynamic "node created" and "link created" lines.
        // note: at the time of writing, NetAnim did not support "link removed" and "node removed" lines
        if (auto notification = dynamic_cast<cPostModuleAddNotification *>(obj)) {
            if (isRelevantModule(notification->module))
                addNode(notification->module);
        }
        else if (auto notification = dynamic_cast<cPostGateConnectNotification *>(obj)) {
            if (isRelevantModule(notification->gate->getOwnerModule()))
                addLink(notification->gate);
        }
    }
}

bool NetAnimTrace::isRelevantModule(cModule *mod)
{
    return mod->getParentModule() == getSimulation()->getSystemModule();
}

void NetAnimTrace::addNode(cModule *mod)
{
    double x, y;
    resolveNodeCoordinates(mod, x, y);
    f << simTime() << " N " << mod->getId() << " " << x << " " << y << "\n";
}

void NetAnimTrace::addLink(cGate *gate)
{
    f << simTime() << " L " << gate->getOwnerModule()->getId() << " " << gate->getNextGate()->getOwnerModule()->getId() << "\n";
}

namespace {
double toDouble(const char *s, double defaultValue)
{
    if (!s || !*s)
        return defaultValue;
    char *end;
    double d = strtod(s, &end);
    return (end && *end) ? 0.0 : d;    // return 0.0 on error, instead of throwing an exception
}
} // namespace {

void NetAnimTrace::resolveNodeCoordinates(cModule *submod, double& x, double& y)
{
    // choose some defaults

    // the standard C++ random generator is used to not modify the result of the simulation:
    x = 600 * (double)rand() / RAND_MAX;
    y = 400 * (double)rand() / RAND_MAX;

    // and be content with them if there is no "p" tag in the display string
    cDisplayString& ds = submod->getDisplayString();
    if (!ds.containsTag("p"))
        return;

    // the following code is based on Tkenv (modinsp.cc, getSubmoduleCoords())

    // read x,y coordinates from "p" tag
    x = toDouble(ds.getTagArg("p", 0), x);
    y = toDouble(ds.getTagArg("p", 1), y);

    double sx = 20;
    double sy = 20;

    const char *layout = ds.getTagArg("p", 2);    // matrix, row, column, ring, exact etc.

    // modify x,y using predefined layouts
    if (!layout || !*layout) {
        // we're happy
    }
    else if (!strcmp(layout, "e") || !strcmp(layout, "x") || !strcmp(layout, "exact")) {
        int dx = toDouble(ds.getTagArg("p", 3), 0);
        int dy = toDouble(ds.getTagArg("p", 4), 0);
        x += dx;
        y += dy;
    }
    else if (!strcmp(layout, "r") || !strcmp(layout, "row")) {
        int dx = toDouble(ds.getTagArg("p", 3), 2 * sx);
        x += submod->getIndex() * dx;
    }
    else if (!strcmp(layout, "c") || !strcmp(layout, "col") || !strcmp(layout, "column")) {
        int dy = toDouble(ds.getTagArg("p", 3), 2 * sy);
        y += submod->getIndex() * dy;
    }
    else if (!strcmp(layout, "m") || !strcmp(layout, "matrix")) {
        int columns = toDouble(ds.getTagArg("p", 3), 5);
        int dx = toDouble(ds.getTagArg("p", 4), 2 * sx);
        int dy = toDouble(ds.getTagArg("p", 5), 2 * sy);
        x += (submod->getIndex() % columns) * dx;
        y += (submod->getIndex() / columns) * dy;
    }
    else if (!strcmp(layout, "i") || !strcmp(layout, "ri") || !strcmp(layout, "ring")) {
        int rx = toDouble(ds.getTagArg("p", 3), (sx + sy) * submod->getVectorSize() / 4);
        int ry = toDouble(ds.getTagArg("p", 4), rx);

        x += (int)floor(rx - rx * sin(submod->getIndex() * 2 * M_PI / submod->getVectorSize()));
        y += (int)floor(ry - ry * cos(submod->getIndex() * 2 * M_PI / submod->getVectorSize()));
    }
    else {
        throw cRuntimeError("Invalid layout `%s' in `p' tag of display string", layout);
    }
}

} // namespace inet

