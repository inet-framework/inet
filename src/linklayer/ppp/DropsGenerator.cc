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

#include "DropsGenerator.h"

Define_Module(DropsGenerator);

void DropsGenerator::initialize()
{
    numPackets = 0;
    numDropped = 0;
    generateFurtherDrops = false;

    WATCH(numPackets);
    WATCH(numDropped);
    WATCH(generateFurtherDrops);

    const char *vector = par("dropsVector");
    parseVector(vector);

    if (dropsVector.size()==0)
        {EV << "DropsGenerator: Empty dropsVector" << endl;}
    else
    {
        EV << "DropsGenerator: dropsVector=" << vector << endl;
        generateFurtherDrops = true;
    }
}

void DropsGenerator::handleMessage(cMessage *msg)
{
    numPackets++;

    if (generateFurtherDrops)
    {
        if (numPackets==dropsVector[0])
        {
            EV << "DropsGenerator: Dropping packet number " << numPackets << " " << msg << endl;
            delete msg;
            numDropped++;
            dropsVector.erase(dropsVector.begin());
            if (dropsVector.size()==0)
            {
                EV << "DropsGenerator: End of dropsVector reached." << endl;
                generateFurtherDrops = false;
            }
        }
        else
            {send(msg, "out");}
    }
    else
        {send(msg, "out");}
}

void DropsGenerator::parseVector(const char *vector)
{
    const char *v = vector;
    while (*v)
    {
        // parse packet numbers
        while (isspace(*v)) v++;
        if (!*v || *v==';') break;
        if (!isdigit(*v))
            throw cRuntimeError("syntax error in dropsVector: packet number expected");
        if (dropsVector.size()>0 && dropsVector.back() >= (unsigned int)atoi(v))
            throw cRuntimeError("syntax error in dropsVector: packet numbers in ascending order expected");

        dropsVector.push_back(atoi(v));
        while (isdigit(*v)) v++;

        // skip delimiter
        while (isspace(*v)) v++;
        if (!*v) break;
        if (*v!=';')
            throw cRuntimeError("syntax error in dropsVector: separator ';' missing");
        v++;
        while (isspace(*v)) v++;
    }
}

void DropsGenerator::finish()
{
    recordScalar("total packets", numPackets);
    recordScalar("total dropped", numDropped);
}
