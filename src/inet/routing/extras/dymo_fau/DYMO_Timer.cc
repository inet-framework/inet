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

#include "inet/routing/extras/dymo_fau/DYMO_Timer.h"
#include <stdexcept>
#include <sstream>

namespace inet {

namespace inetmanet {

DYMO_Timer::DYMO_Timer(cSimpleModule* parent, const char *name, simtime_t interval) :
        parent(parent), interval(interval), active(false)
{
    this->name = strdup(name);
}

DYMO_Timer::~DYMO_Timer()
{
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

bool DYMO_Timer::isRunning() const
{
    return active && (expiresAt > simTime());
}

bool DYMO_Timer::stopWhenExpired()
{
    if (active && (expiresAt <= simTime()))
    {
        active = false;
        return true;
    }
    return false;
}

void DYMO_Timer::start(simtime_t interval)
{
    if (interval != 0) this->interval = interval;
    if (this->interval == 0) throw std::runtime_error("Tried starting DYMO_Timer without or with zero-length interval");
    expiresAt = simTime() + this->interval;
    active = true;
}

void DYMO_Timer::cancel()
{
    active = false;
}

simtime_t DYMO_Timer::getInterval() const
{
    return interval;
}

std::ostream& operator<<(std::ostream& os, const DYMO_Timer& o)
{
    os << o.info();
    return os;
}

} // namespace inetmanet

} // namespace inet

