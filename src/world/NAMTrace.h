//
// Copyright (C) 2005 Andras Varga
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

#ifndef __INET_NAMTRACE_H
#define __INET_NAMTRACE_H

#include <fstream>
#include <omnetpp.h>
#include "INETDefs.h"

/**
 * Provides a central place for storing the output stream of an ns2 nam trace.
 *
 * Only one NAMTrace module should be in a network (or, per subnetwork), and
 * hosts/routers should contain a NAMTraceWriter module each.
 * Every NAMTraceWriters write to an output stream which they obtain from
 * the out() method of the shared NAMTrace module.
 *
 * See NED file for more info.
 *
 * @author Andras Varga
 */
class INET_API NAMTrace : public cSimpleModule
{
  protected:
    std::ofstream *nams;

    int lastnamid;
    std::map<int,int> modid2namid;

  public:
    NAMTrace();
    virtual ~NAMTrace();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

  public:
    /**
     * Assign a nam ID to the given getModule(host or router).
     * -1 means auto-assigned ID.
     */
    virtual int assignNamId(cModule *node, int namid=-1);

    /**
     * Returns the nam ID of the given getModule(host or router). assignNamId() must
     * have been called for the given module before, at least with -1 (auto-ID).
     */
    virtual int getNamId(cModule *node) const;

    /**
     * Returns true if nam trace recording is enabled (filename was not "").
     */
    virtual bool isEnabled() const {return nams!=NULL;}

    /**
     * Returns the stream to which the trace events can be written.
     */
    std::ostream& out() {ASSERT(nams!=NULL); return *nams;}
};

#endif
