//
// Copyright (C) 2006 Autonomic Networking Group,
// Department of Computer Science 7, University of Erlangen, Germany
//
// Author: Isabel Dietrich
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

#include "TrafGenPar.h"
#include <string>

//Define_Module(TrafGen);

/////////////////////////////// PUBLIC ///////////////////////////////////////

//============================= LIFECYCLE ===================================
/**
 * Initialization routine
 */

void TrafGenPar::initialize(int aStage)
{
    cSimpleModule::initialize(aStage);

    if (0 == aStage)
    {
        ev << "initializing TrafGen..." << endl;

        mpSendMessage = new cMessage("SendMessage");
        mpOnOffSwitch = new cMessage("onOffSwitch");

        if (par("isSink"))
        {
            // no traffic is to be sent by this node
            return;
        }

        // read all the parameters from the xml file
        if (FirstPacketTime() < 0)
        {
            // no traffic is to be sent by this node
            return;
        }

        // the onOff-traffic timer is scheduled
        // only if the parameters for onOff-traffic are present
        if ((OnIntv() > 0) && (OffIntv() > 0))
        {
            scheduleAt(simTime() + OnIntv(), mpOnOffSwitch);
            mOnOff = TRAFFIC_ON;
        }

        // if the offInterArrivalTime attribute is present: packets are sent during the off interval too
        if ((mOnOff == TRAFFIC_ON) && (OffInterDepartureTime() > 0))
        {
            mOffTraffic = true;
        }
        else
        {
            mOffTraffic = false;
        }

        // if the onIdenticalTrafDest attribute is present: packets are
        // sent to the same destination during on intervals
        if (mOnOff == TRAFFIC_ON)
        {
            mOnIdenticalDest = par("onIdenticalTrafDest");
        }
        else
        {
            mOnIdenticalDest = false;
        }

        mDestination = par("trafDest").stringValue();
        if (mDestination == std::string("-1"))
        {
            mDestination = "BROADCAST"; // Broadcast
        }

        if (mOnIdenticalDest)
        {
            mCurrentOnDest = calculateDestination();
        }

        if (FirstPacketTime() < 0)
            error("TrafficGenerator, attribute firstPacketTime: time < 0 is not legal");

        scheduleAt(simTime() + FirstPacketTime(), mpSendMessage);

        WATCH(mOnOff);
    }
}

/**
 * Function called before module destruction
 * Used for recording statistics and deallocating memory
 */
void TrafGenPar::finish()
{
    cancelEvent(mpSendMessage);
    delete mpSendMessage;
    cancelEvent(mpOnOffSwitch);
    delete mpOnOffSwitch;
}

//============================= OPERATIONS ===================================
/**
 * Function called whenever a message arrives at the module
 */
void TrafGenPar::handleMessage(cMessage * apMsg)
{
    if (apMsg->isSelfMessage())
    {
        handleSelfMsg(apMsg);
    }
    else
    {
        handleLowerMsg(apMsg);
    }
}

/**
 * @return The time when the first packet should be scheduled
 */
double TrafGenPar::FirstPacketTime()
{
    return par("firstPacketTime").doubleValue();
}

/**
 * @return The time between two subsequent packets
 *
 * WARNING: the return value should not be buffered, as it can change with each
 *    call in case a distribution function is specified as simulation parameter!
 */
double TrafGenPar::InterDepartureTime()
{
    return par("interDepartureTime").doubleValue();
}

/**
 * @return The packet length
 *
 * WARNING: the return value should not be buffered, as it can change with each
 *    call in case a distribution function is specified as simulation parameter!
 */
long TrafGenPar::PacketSize()
{
    return par("packetSize").longValue();
}

double TrafGenPar::OnIntv()
{
    return par("onLength").doubleValue();
}

double TrafGenPar::OffIntv()
{
    return par("offLength").doubleValue();
}

double TrafGenPar::OffInterDepartureTime()
{
    return par("offInterDepartureTime").doubleValue();
}

/////////////////////////////// PROTECTED  ///////////////////////////////////

//============================= OPERATIONS ===================================

void TrafGenPar::handleLowerMsg(cMessage * apMsg)
{
    // only relevant for the sink
    delete apMsg;
}

/**
 * Handles self messages (i.e. timer)
 * Two timer types:
 * - mpOnOffSwitch for switching between the on and off state of the generator
 * - mpSendMessage for sending a new message
 * @param pMsg a pointer to the just received message
 */
void TrafGenPar::handleSelfMsg(cMessage * apMsg)
{
    // handle the switching between on and off periods of the generated traffic
    // the values for offIntv, onIntv and interDepartureTime are evaluated each
    // time, in case a distribution function is specified
    if (apMsg == mpOnOffSwitch)
    {
        if (mOnOff == TRAFFIC_ON)
        {
            ev << "switch traffic off" << endl;
            mOnOff = TRAFFIC_OFF;
            scheduleAt(simTime() + OffIntv(), mpOnOffSwitch);
            cancelEvent(mpSendMessage);
            if (mOffTraffic)
            {
                scheduleAt(simTime() + OffInterDepartureTime(), mpSendMessage);
            }
        }
        else if (mOnOff == TRAFFIC_OFF)
        {
            ev << "switch traffic on" << endl;
            mOnOff = TRAFFIC_ON;
            cancelEvent(mpSendMessage);
            scheduleAt(simTime() + OnIntv(), mpOnOffSwitch);
            scheduleAt(simTime() + InterDepartureTime(), mpSendMessage);

            // if identical traffic destinations inside the on interval are
            // required, calculate the destination now!
            if (mOnIdenticalDest)
            {
                mCurrentOnDest = calculateDestination();
            }
        }

    }
    // handle the sending of a new message
    else if (apMsg == mpSendMessage)
    {
        cPacket *p_traffic_msg = new cPacket("TrafGen Message");

        // calculate the destination and send the message:

        if (mOnOff == TRAFFIC_ON && mOnIdenticalDest)
        {
            ev << "sending message to " << mCurrentOnDest.c_str() << endl;
            SendTraf(p_traffic_msg, mCurrentOnDest.c_str());
        }
        else
        {
            std::string dest = calculateDestination();
            ev << "sending message to " << dest.c_str() << endl;
            SendTraf(p_traffic_msg, dest.c_str());
        }

        // schedule next event
        // interDepartureTime is evaluated each time,
        // in case a distribution function is specified
        if (mOffTraffic && mOnOff == TRAFFIC_OFF)
            scheduleAt(simTime() + OffInterDepartureTime(), mpSendMessage);
        else
            scheduleAt(simTime() + InterDepartureTime(), mpSendMessage);
    }
}

/**
 * This function calculates the host to be used as traffic destination
 * @return string containing the destination host name
 */
std::string TrafGenPar::calculateDestination()
{
    if (mDestination.find_first_of('*') == std::string::npos)
    {
        // no asterisk in the destination, no calculation needed
        return mDestination;
    }
    else
    {
        // asterisk present, find out how many hosts with the specified name
        // there are and randomly (uniform) pick one
        std::string s = mDestination;
        int index = s.find_first_of('*');
        s.replace(index, 1, "0");
        int size = simulation.getModuleByPath(s.c_str())->size();
        s = s.substr(0, index - 1);
        size = intuniform(0, size - 1);
        std::string dest(s);
        dest.append("[");
        char temp[10];
        sprintf(temp, "%i", size);
        dest.append(temp);
        dest.append("]");
        return dest;
    }
}
