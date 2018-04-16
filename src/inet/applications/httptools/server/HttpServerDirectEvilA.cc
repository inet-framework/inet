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

#include "inet/applications/httptools/server/HttpServerDirectEvilA.h"

namespace inet {

namespace httptools {

Define_Module(HttpServerDirectEvilA);

void HttpServerDirectEvilA::initialize(int stage)
{
    HttpServerDirect::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        badLow = par("minBadRequests");
        badHigh = par("maxBadRequests");

        EV_INFO << "Badguy " << hostName << " was initialized to launch an attack on www.good.com" << endl;
        EV_INFO << "Minimum " << badLow << " and maximum " << badHigh << " bad requests for each hit." << endl;
    }
}

std::string HttpServerDirectEvilA::generateBody()
{
    int numImages = badLow + (int)uniform(0, badHigh - badLow);         //FIXME use intuniform(...)
    double rndDelay;
    std::string result;

    char tempBuf[128];
    for (int i = 0; i < numImages; i++) {
        rndDelay = 10.0 + uniform(0, 2.0);
        sprintf(tempBuf, "IMG%.4d.jpg;%s;%f\n", i, "www.good.com", rndDelay);
        result.append(tempBuf);
    }

    return result;
}

} // namespace httptools

} // namespace inet

