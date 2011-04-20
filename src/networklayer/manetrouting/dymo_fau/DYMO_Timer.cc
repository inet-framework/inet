/*
 *  Copyright (C) 2006,2007 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "DYMO_Timer.h"
#include <stdexcept>
#include <sstream>

DYMO_Timer::DYMO_Timer(cSimpleModule* parent, std::string name, simtime_t interval) : parent(parent), interval(interval), active(false)
{
    this->name = strdup(name.c_str());
    message = new DYMO_Timeout(this->name, 1 /* msg kind, to give the msg a green color */);
}

DYMO_Timer::~DYMO_Timer()
{
    //cancel();
    //delete message;
    parent->cancelAndDelete(message);
    free(this->name);
}

const char* DYMO_Timer::getFullName() const
{
    return name;
}

std::string DYMO_Timer::info() const
{
    if (!active)
    {
        return "inactive";
    }
    std::ostringstream ss;
    ss << (expiresAt - (simTime())) << "s left";
    return ss.str();
}

std::string DYMO_Timer::detailedInfo() const
{
    return info();
}

bool DYMO_Timer::isRunning()
{
    if (active && (expiresAt > simTime()))
    {
        return true;
    }
    return false;
}

bool DYMO_Timer::isExpired()
{
    if (active && (expiresAt <= simTime()))
    {
        parent->cancelEvent(message);
        active = false;
        return true;
    }
    return false;
}

void DYMO_Timer::start(simtime_t interval)
{
    if (interval != 0) this->interval = interval;
    if (this->interval == 0) throw std::runtime_error("Tried starting DYMO_Timer without or with zero-length interval");
    if (active) parent->cancelEvent(message);
    expiresAt = simTime() + this->interval;
    active = true;
    parent->scheduleAt(expiresAt, message);
}

void DYMO_Timer::cancel()
{
    if (active) parent->cancelEvent(message);
    active = false;
}

bool DYMO_Timer::owns(const cMessage* message) const
{
    return (message == this->message);
}

simtime_t DYMO_Timer::getInterval() const
{
    return interval;
}

std::ostream& operator<< (std::ostream& os, const DYMO_Timer& o)
{
    os << o.info();
    return os;
}

