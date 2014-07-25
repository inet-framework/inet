//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
    int numResources = badLow + (int)uniform(0, badHigh - badLow);
    double rndDelay;
    std::string result;

    char tempBuf[128];
    int refSize;
    for (int i = 0; i < numResources; i++) {
        rndDelay = 10.0 + uniform(0, 2.0);
        refSize = (int)uniform(500, 1000);    // The random size represents a random reference string length
        sprintf(tempBuf, "TEXT%.4d.txt;%s;%f;%s;%d\n", i, "www.good.com", rndDelay, "TRUE", refSize);
        result.append(tempBuf);
    }

    return result;
}

} // namespace httptools

} // namespace inet

