//
// Copyright (C) 2006 Autonomic Networking Group,
// Department of Computer Science 7, University of Erlangen, Germany
//
// Author: Isabel Dietrich
//
// 2009 Alfonso Ariza.
// Use the par() (omnet 4.0)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
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

/** Documentation copied from the wiki...

The TrafGen module (in the folder Isabel/Applications/TrafficGenerator) is an all-purpose traffic generator which can be used with the INET framework. Using a flexible xml-based parameter structure, the generator can model all kinds of traffic, for example cbr traffic, on-off-traffic or exponentially distributed traffic. Traffic patterns can also be changed dynamically during the simulation.

'''Traffic Patterns:'''

The generator supports (among others) cbr traffic, on-off-traffic and exponentially distributed traffic. Please see the parameter section for an overview of exactly what kinds of traffic can be modeled with the generator.
Please also note that trace files are not supported.

'''Usage:'''

The class TrafGen is an abstract base class. This means you HAVE to subclass from it in order to use it in a simulation. The only element required in that subclass is the function sendTraf(), which receives the message to be sent and the destination in a character string as parameters and has to translate the destination to an address its lower layer can understand.

An example how this can be achieved can be found in the folder Isabel/Applications/Mobile/DymoTestApp.cpp.

'''Parameter configuration:'''

The traffic generator understands the following parameters:
* packet size in byte: can be a constant number or a distribution (i.e. exponential(10)), just like supported in omnetpp.ini files (section 3.7.8 of the OMNeT++ manual)
* inter departure times in seconds: can be a constant number or a distribution
* first packet time in seconds: can be a constant number or a distribution
* destination: can be any host that is present in your simulation, e.g. host[5] or MobileHost[2]. Wildcards are only supported inside the array brackets (e.g. host[*]). If a wildcard is present, a random member of the specified host array is chosen. You can use a destination of -1 to indicate a broadcast.
* on and off length in seconds: can be constant numbers or distributions. These parameters are only necessary if the generated traffic should be on-off-traffic.
* inter departure time used during off-traffic periods: can be a constant number or a distribution. This parameter is not mandatory. If it is left out, no traffic will be generated during off periods
* identical traffic destinations during on intervals: if this parameter is set to true, all packets generated during an on interval will be sent to the same destination host. At the beginning of the next on interval, the destination is evaluated again. This parameter is not mandatory. If it is left out, a value of false is assumed.

Multiple traffic patterns can be defined in one xml file by varying the id parameter. The pattern to be used in a specific application can be specified in omnetpp.ini using the parameter defaultTrafConfigId. The traffic pattern can also be changed during the simulation by calling setParams().

Nodes sending no traffic can be achieved by two methods:
* setting firstPacketTime to -1 in the xml file
* setting the parameter defaultTrafConfigId to -1 in the omnetpp.ini file

*/

/**
 * @short Abstract base class for a universal traffic generator
 * @author Isabel Dietrich
*/

#ifndef TRAFFIC_GENERATOR_H
#define TRAFFIC_GENERATOR_H

// SYSTEM INCLUDES
#include <omnetpp.h>
#include <string>

class TrafGenPar : public cSimpleModule
{
  public:
    // LIFECYCLE
    // this takes care of constructors and destructors
    //Module_Class_Members(TrafGen, cSimpleModule, 0);
    virtual void initialize(int);
    virtual void finish();

    // OPERATIONS
    virtual void handleMessage(cMessage*);
    double FirstPacketTime();
    double InterDepartureTime();
    long PacketSize();
    double OnIntv();

    double OffIntv();
    double OffInterDepartureTime();

    enum TrafficStateType
    {
        TRAFFIC_ON,
        TRAFFIC_OFF
    };

  protected:

    // OPERATIONS
    virtual void handleSelfMsg(cMessage*);
    virtual void handleLowerMsg(cMessage*);

    virtual void SendTraf(cPacket*, const char*) = 0;
    std::string calculateDestination();

  private:
    // MEMBER VARIABLES
    bool mOffTraffic;
    bool mOnIdenticalDest;
    bool mOnOff;
    std::string mDestination;

    std::string mCurrentOnDest;

    cMessage *mpSendMessage;
    cMessage *mpOnOffSwitch;
};

#endif
