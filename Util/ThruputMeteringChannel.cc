//
// Copyright (C) 2005 Andras Varga
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

#include "ThruputMeteringChannel.h"

Register_Class(ThruputMeteringChannel);

ThruputMeteringChannel::ThruputMeteringChannel(const char *name) : cBasicChannel(name)
{
    count = 0;
}

ThruputMeteringChannel::ThruputMeteringChannel(const ThruputMeteringChannel& ch) : cBasicChannel()
{
    setName(ch.name());
    operator=(ch);
}

ThruputMeteringChannel::~ThruputMeteringChannel()
{
}

ThruputMeteringChannel& ThruputMeteringChannel::operator=(const ThruputMeteringChannel& ch)
{
    if (this==&ch) return *this;
    cBasicChannel::operator=(ch);
    count = ch.count;
    return *this;
}


bool ThruputMeteringChannel::deliver(cMessage *msg, simtime_t t)
{
    bool ret = cBasicChannel::deliver(msg, t);

    count++;
    fromGate()->displayString().setTagArg("t",0, count);

    return ret;
}

