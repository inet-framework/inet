//
// Copyright (C) 2011 Philipp Berndt
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

#include "cdataratechannel2.h"

Define_Channel(cDatarateChannel2);

cDatarateChannel2::cDatarateChannel2(const char *name)
: cDatarateChannel(name)
{}

void cDatarateChannel2::setDelay(double d)
{
    cDatarateChannel::setDelay(d);
    rereadPars();
}

void cDatarateChannel2::setDatarate(double d)
{
    cDatarateChannel::setDatarate(d);
    rereadPars();
}

void cDatarateChannel2::setBitErrorRate(double d)
{
    cDatarateChannel::setBitErrorRate(d);
    rereadPars();
}

void cDatarateChannel2::setPacketErrorRate(double d)
{
    cDatarateChannel::setPacketErrorRate(d);
    rereadPars();
}

void cDatarateChannel2::setDisabled(bool d)
{
    cDatarateChannel::setDisabled(d);
    rereadPars();
}
