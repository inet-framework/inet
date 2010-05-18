//
// Copyright (C) 2010 Zoltan Bojthe
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

#ifndef __INET_ITCP_H
#define __INET_ITCP_H

#include <map>
#include <set>
#include <omnetpp.h>
#include "IPvXAddress.h"


class TCPConnection;
class TCPSegment;

// macro for normal ev<< logging (Note: deliberately no parens in macro def)
#define tcpEV (ev.disable_tracing||TCP::testing)?ev:ev

// macro for more verbose ev<< logging (Note: deliberately no parens in macro def)
#define tcpEV2 (ev.disable_tracing||TCP::testing||!TCP::logverbose)?ev:ev

// testingEV writes log that automated test cases can check (*.test files)
#define testingEV (ev.disable_tracing||!TCP::testing)?ev:ev





/**
 * Base class for TCP implementations.
 */
class INET_API ITCP : public cSimpleModule
{
  public:
    /** Constructor */
    ITCP() {}

    /** Destructor */
    virtual ~ITCP();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

  public:
};

#endif


