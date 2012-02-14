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

#ifndef __INET_NETANIMTRACEWRITER_H
#define __INET_NETANIMTRACEWRITER_H

#include <fstream>
#include <INETDefs.h>



/**
 * Records a NetAnim trace. See NED file for more information.
 *
 * @author andras
 */
class INET_API NetAnimTrace : public cSimpleModule, protected cListener
{
  protected:
    static simsignal_t messageSentSignal;
    static simsignal_t mobilityStateChangedSignal;
    std::ofstream f;
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
    virtual void dump();
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
    virtual bool isRelevantModule(cModule *mod);
    virtual void resolveNodeCoordinates(cModule *mod, double& x, double& y);
    virtual void addNode(cModule *mod);
    virtual void addLink(cGate *gate);
};

#endif  // header guard


