//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/filter/OrdinalBasedDropper.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

Define_Module(OrdinalBasedDropper);

void OrdinalBasedDropper::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        numPackets = 0;
        numDropped = 0;
        generateFurtherDrops = false;

        WATCH(numPackets);
        WATCH(numDropped);
        WATCH(generateFurtherDrops);

        const char *vector = par("dropsVector");
        parseVector(vector);

        if (dropsVector.size() == 0)
            EV_WARN << "Empty dropsVector" << EV_ENDL;
        else {
            EV_DEBUG << EV_FIELD(dropsVector, vector) << EV_ENDL;
            generateFurtherDrops = true;
        }
    }
}

bool OrdinalBasedDropper::matchesPacket(const Packet *packet) const
{
    return !generateFurtherDrops || numPackets != dropsVector[0];
}

void OrdinalBasedDropper::processPacket(Packet *packet)
{
    numPackets++;
}

void OrdinalBasedDropper::dropPacket(Packet *packet)
{
    EV_DEBUG << "Dropping packet" << EV_FIELD(ordinalNumber, numPackets) << EV_FIELD(packet) << EV_ENDL;
    numPackets++;
    numDropped++;
    dropsVector.erase(dropsVector.begin());
    if (dropsVector.size() == 0) {
        EV_DEBUG << "End of dropsVector reached" << EV_ENDL;
        generateFurtherDrops = false;
    }
    PacketFilterBase::dropPacket(packet);
}

void OrdinalBasedDropper::parseVector(const char *vector)
{
    const char *v = vector;
    while (*v) {
        // parse packet numbers
        while (isspace(*v))
            v++;
        if (!*v || *v == ';')
            break;
        if (!isdigit(*v))
            throw cRuntimeError("Syntax error in dropsVector: packet number expected");
        if (dropsVector.size() > 0 && dropsVector.back() >= (unsigned int)atoi(v))
            throw cRuntimeError("Syntax error in dropsVector: packet numbers in ascending order expected");

        dropsVector.push_back(atoi(v));
        while (isdigit(*v))
            v++;

        // skip delimiter
        while (isspace(*v))
            v++;
        if (!*v)
            break;
        if (*v != ';')
            throw cRuntimeError("Syntax error in dropsVector: separator ';' missing");
        v++;
        while (isspace(*v))
            v++;
    }
}

} // namespace queueing
} // namespace inet

