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

#include <string>

#include "applications/trafgen/TrafGen.h"

//Define_Module(TrafGen);

/////////////////////////////// PUBLIC ///////////////////////////////////////

//============================= LIFECYCLE ===================================
/**
 * Initialization routine
 */

TrafGen::TrafGen()
        :
        mpSendMessage(NULL),
        mpOnOffSwitch(NULL)
{
}

TrafGen::~TrafGen()
{
    cancelAndDelete(mpSendMessage);
    cancelAndDelete(mpOnOffSwitch);
}

void TrafGen::initialize(int aStage)
{
    cSimpleModule::initialize(aStage);

    if (0 == aStage)
    {
        ev << "initializing TrafGen..." << endl;

        mpSendMessage = new cMessage("SendMessage");
        mpOnOffSwitch = new cMessage("onOffSwitch");

        mDefaultTrafConfigId = par("defaultTrafConfigId");

        if (mDefaultTrafConfigId == -1)
        {
            // no traffic is to be sent by this node
            return;
        }

        char id[5];
        sprintf(id, "%d", mDefaultTrafConfigId);

        // read all the parameters from the xml file
        cXMLElement* rootelement = par("trafConfig").xmlValue();

        try
        {
            mPacketSize.parse(rootelement->getElementById(id)->getAttribute("packetSize"));
            mInterDepartureTime.parse(rootelement->getElementById(id)->getAttribute("interDepartureTime"));
            mFirstPacketTime.parse(rootelement->getElementById(id)->getAttribute("firstPacketTime"));
            if (mFirstPacketTime.longValue(this) == -1)
            {
                // no traffic is to be sent by this node
                return;
            }

            // the onOff-traffic timer is scheduled
            // only if the parameters for onOff-traffic are present
            if (rootelement->getElementById(id)->getAttribute("onLength") != NULL)
            {
                mOnIntv.parse(rootelement->getElementById(id)->getAttribute("onLength"));
                mOffIntv.parse(rootelement->getElementById(id)->getAttribute("offLength"));
                if (mOnIntv.doubleValue(this) < 0)
                    error("TrafficGenerator, attribute onLength: interval length < 0 is not legal");
                if (mOffIntv.doubleValue(this) < 0)
                    error("TrafficGenerator, attribute offLength: interval length < 0 is not legal");

                scheduleAt(simTime() + mOnIntv.doubleValue(this), mpOnOffSwitch);
                mOnOff = TRAFFIC_ON;
            }

            // if the offInterArrivalTime attribute is present: packets are sent during the off interval too
            if (mOnOff == TRAFFIC_ON
                    && rootelement->getElementById(id)->getAttribute("offInterDepartureTime") != NULL)
            {
                mOffTraffic = true;
                mOffInterDepartureTime.parse(rootelement->getElementById(id)->
                                             getAttribute("offInterDepartureTime"));
            }
            else
            {
                mOffTraffic = false;
            }

        }
        catch (std::runtime_error e)
        {
            error((std::string("error reading parameters from xml file: ") + e.what()).c_str());
        }

        // if the onIdenticalTrafDest attribute is present: packets are
        // sent to the same destination during on intervals
        if (mOnOff == TRAFFIC_ON
                && rootelement->getElementById(id)->getAttribute("onIdenticalTrafDest") != NULL)
        {
            cMsgPar temp;
            if (!temp.parse(rootelement->getElementById(id)->getAttribute("onIdenticalTrafDest")))
            {
                error("wrong value in xml file, attribute onIdenticalTrafDest");
            }
            mOnIdenticalDest = temp.boolValue();
        }
        else
        {
            mOnIdenticalDest = false;
        }

        const char *destination = rootelement->getElementById(id)->getAttribute("trafDest");
        mDestination = destination ? destination : "";
        if (mDestination == std::string("-1"))
        {
            mDestination = "BROADCAST"; // Broadcast
        }

        if (mOnIdenticalDest)
        {
            mCurrentOnDest = calculateDestination();
        }

        if (mFirstPacketTime.doubleValue(this) < 0)
            error("TrafficGenerator, attribute firstPacketTime: time < 0 is not legal");

        scheduleAt(simTime() + mFirstPacketTime.doubleValue(this), mpSendMessage);

        WATCH(mOnOff);
    }
}

/**
 * Function called before module destruction
 * Used for recording statistics and deallocating memory
 */
void TrafGen::finish()
{
}

//============================= OPERATIONS ===================================
/**
 * Function called whenever a message arrives at the module
 */
void TrafGen::handleMessage(cMessage* apMsg)
{
    if (apMsg->isSelfMessage())
    {
        handleSelfMsg(apMsg);
    }
    else
    {
        handleLowerMsg(check_and_cast<cPacket*>(apMsg));
    }
}

/**
 * @return The time when the first packet should be scheduled
 */
double TrafGen::FirstPacketTime()
{
    return mFirstPacketTime.doubleValue(this);
}

/**
 * @return The time between two subsequent packets
 *
 * WARNING: the return value should not be buffered, as it can change with each
 *    call in case a distribution function is specified as simulation parameter!
 */
double TrafGen::InterDepartureTime()
{
    return mInterDepartureTime.doubleValue(this);
}

/**
 * @return The packet length
 *
 * WARNING: the return value should not be buffered, as it can change with each
 *    call in case a distribution function is specified as simulation parameter!
 */
long TrafGen::PacketSize()
{
    return mPacketSize.longValue(this);
}

/////////////////////////////// PROTECTED  ///////////////////////////////////

//============================= OPERATIONS ===================================

void TrafGen::handleLowerMsg(cPacket* apMsg)
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
void TrafGen::handleSelfMsg(cMessage* apMsg)
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
            scheduleAt(simTime() + mOffIntv.doubleValue(this), mpOnOffSwitch);
            cancelEvent(mpSendMessage);
            if (mOffTraffic)
            {
                scheduleAt(simTime() + mOffInterDepartureTime.doubleValue(this), mpSendMessage);
            }
        }
        else if (mOnOff == TRAFFIC_OFF)
        {
            ev << "switch traffic on" << endl;
            mOnOff = TRAFFIC_ON;
            cancelEvent(mpSendMessage);
            scheduleAt(simTime() + mOnIntv.doubleValue(this), mpOnOffSwitch);
            scheduleAt(simTime() + mInterDepartureTime.doubleValue(this), mpSendMessage);

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
        cPacket* p_traffic_msg = new cPacket("TrafGen Message");

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
            scheduleAt(simTime() + mOffInterDepartureTime.doubleValue(this), mpSendMessage);
        else
            scheduleAt(simTime() + mInterDepartureTime.doubleValue(this), mpSendMessage);
    }
}

/**
 * This function calculates the host to be used as traffic destination
 * @return string containing the destination host name
 */
std::string TrafGen::calculateDestination()
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

/**
 * This function can be used to dynamically change the traffic pattern
 * during the simulation.
 * The function also takes care of rescheduling the send message timer.
 *
 * @param newTrafficPattern designates the config id from the xml traffic file
 */
void TrafGen::setParams(int aNewTrafficPattern)
{
    cXMLElement* rootelement = par("trafConfig").xmlValue();
    char buf[4];
    sprintf(buf, "%d", aNewTrafficPattern);
    try
    {
        mPacketSize.parse(rootelement->getElementById(buf)->getAttribute("packetSize"));
        mInterDepartureTime.parse(rootelement->getElementById(buf)->getAttribute("interDepartureTime"));
        mFirstPacketTime.parse(rootelement->getElementById(buf)->getAttribute("firstPacketTime"));
        if (rootelement->getElementById(buf)->getAttribute("onLength") != NULL)
        {
            mOnIntv.parse(rootelement->getElementById(buf)->getAttribute("onLength"));
            mOnIntv.parse(rootelement->getElementById(buf)->getAttribute("offLength"));
            cancelEvent(mpOnOffSwitch);
            scheduleAt(simTime() + mOnIntv.doubleValue(this), mpOnOffSwitch);
            mOnOff = TRAFFIC_ON;
        }
        else
        {
            cancelEvent(mpOnOffSwitch);
        }

        // if the offInterArrivalTime attribute is present: packets are sent during the off interval too
        if (mOnOff == TRAFFIC_ON
                && rootelement->getElementById(buf)->getAttribute("offInterDepartureTime") != NULL)
        {
            mOffTraffic = true;
            mOffInterDepartureTime.parse(rootelement->getElementById(buf)->
                                         getAttribute("offInterDepartureTime"));
        }
        else
        {
            mOffTraffic = false;
        }

        // if the onIdenticalTrafDest attribute is present: packets are
        // sent to the same destination during on intervals
        if (mOnOff == TRAFFIC_ON
                && rootelement->getElementById(buf)->getAttribute("onIdenticalTrafDest") != NULL)
        {
            cMsgPar temp;
            temp.parse(rootelement->getElementById(buf)->getAttribute("onIdenticalTrafDest"));
            mOnIdenticalDest = temp.boolValue();
        }
        else
        {
            mOnIdenticalDest = false;
        }
    }
    catch (std::runtime_error e)
    {
        error((std::string("error reading parameters from xml file: ") + e.what()).c_str());
    }

    mDestination = rootelement->getElementById(buf)->getAttribute("trafDest");
    if (mDestination == std::string("-1"))
    {
        mDestination = "BROADCAST"; // Broadcast
    }

    if (mOnIdenticalDest)
    {
        mCurrentOnDest = calculateDestination();
    }

    // reschedule the send message timer
    if (simTime() != 0)
    {
        cancelEvent(mpSendMessage);
        scheduleAt(simTime() + InterDepartureTime(), mpSendMessage);
    }
    else
    {
        cancelEvent(mpSendMessage);
        scheduleAt(simTime() + FirstPacketTime(), mpSendMessage);
    }
}
