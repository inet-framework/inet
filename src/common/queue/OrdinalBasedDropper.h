//
// Copyright (C) 2009 Thomas Reschka
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


#ifndef __INET_ORDINALBASEDDROPPER_H
#define __INET_ORDINALBASEDDROPPER_H

#include <vector>

#include "INETDefs.h"


/**
 * Ordinal Based Dropper module.
 */
class INET_API OrdinalBasedDropper : public cSimpleModule
{
  protected:
    unsigned int numPackets;
    unsigned int numDropped;

    static simsignal_t rcvdPkSignal;
    static simsignal_t sentPkSignal;
    static simsignal_t dropPkSignal;

    bool generateFurtherDrops;
    std::vector<unsigned int> dropsVector;

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void parseVector(const char *vector);
    virtual void finish();
};

#endif
