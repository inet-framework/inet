//
// Copyright (C) 2009 Thomas Reschka
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

#include "inet/common/queue/OrdinalBasedDuplicator.h"

namespace inet {

Define_Module(OrdinalBasedDuplicator);

simsignal_t OrdinalBasedDuplicator::rcvdPkSignal = registerSignal("rcvdPk");
simsignal_t OrdinalBasedDuplicator::sentPkSignal = registerSignal("sentPk");
simsignal_t OrdinalBasedDuplicator::duplPkSignal = registerSignal("duplPk");

void OrdinalBasedDuplicator::initialize()
{
    numPackets = 0;
    numDuplicated = 0;
    generateFurtherDuplicates = false;

    WATCH(numPackets);
    WATCH(numDuplicated);
    WATCH(generateFurtherDuplicates);

    const char *vector = par("duplicatesVector");
    parseVector(vector);

    if (duplicatesVector.size() == 0) {
        EV << "DuplicatesGenerator: Empty duplicatesVector" << endl;
    }
    else {
        EV << "DuplicatesGenerator: duplicatesVector=" << vector << endl;
        generateFurtherDuplicates = true;
    }
}

void OrdinalBasedDuplicator::handleMessage(cMessage *msg)
{
    numPackets++;

    emit(rcvdPkSignal, msg);

    if (generateFurtherDuplicates) {
        if (numPackets == duplicatesVector[0]) {
            EV << "DuplicatesGenerator: Duplicating packet number " << numPackets << " " << msg << endl;
            cMessage *dupmsg = msg->dup();
            emit(duplPkSignal, dupmsg);
            emit(sentPkSignal, dupmsg);
            send(dupmsg, "out");
            numDuplicated++;
            duplicatesVector.erase(duplicatesVector.begin());
            if (duplicatesVector.size() == 0) {
                EV << "DuplicatesGenerator: End of duplicatesVector reached." << endl;
                generateFurtherDuplicates = false;
            }
        }
    }
    emit(sentPkSignal, msg);
    send(msg, "out");
}

void OrdinalBasedDuplicator::parseVector(const char *vector)
{
    const char *v = vector;
    while (*v) {
        // parse packet numbers
        while (isspace(*v))
            v++;
        if (!*v || *v == ';')
            break;
        if (!isdigit(*v))
            throw cRuntimeError("Syntax error in duplicatesVector: packet number expected");
        if (duplicatesVector.size() > 0 && duplicatesVector.back() >= (unsigned int)atoi(v))
            throw cRuntimeError("Syntax error in duplicatesVector: packet numbers in ascending order expected");

        duplicatesVector.push_back(atoi(v));
        while (isdigit(*v))
            v++;

        // skip delimiter
        while (isspace(*v))
            v++;
        if (!*v)
            break;
        if (*v != ';')
            throw cRuntimeError("Syntax error in duplicatesVector: separator ';' missing");
        v++;
        while (isspace(*v))
            v++;
    }
}

void OrdinalBasedDuplicator::finish()
{
}

} // namespace inet

