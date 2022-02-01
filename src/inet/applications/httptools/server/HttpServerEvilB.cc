//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
//
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "inet/applications/httptools/server/HttpServerEvilB.h"

namespace inet {

namespace httptools {

Define_Module(HttpServerEvilB);

void HttpServerEvilB::initialize(int stage)
{
    HttpServer::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        badLow = par("minBadRequests");
        badHigh = par("maxBadRequests");

        EV_INFO << "Badguy " << hostName << " was initialized to launch an attack on www.good.com" << endl;
        EV_INFO << "Minimum " << badLow << " and maximum " << badHigh << " bad requests for each hit." << endl;
    }
}

std::string HttpServerEvilB::generateBody()
{
    int numResources = badLow + (int)uniform(0, badHigh - badLow); // FIXME use intuniform(...)
    double rndDelay;
    std::string result;

    char tempBuf[128];
    int refSize;
    for (int i = 0; i < numResources; i++) {
        rndDelay = 10.0 + uniform(0, 2.0);
        refSize = (int)uniform(500, 1000); // The random size represents a random reference string length         //FIXME use intuniform(...)
        sprintf(tempBuf, "TEXT%.4d.txt;%s;%f;%s;%d\n", i, "www.good.com", rndDelay, "TRUE", refSize);
        result.append(tempBuf);
    }

    return result;
}

} // namespace httptools

} // namespace inet

