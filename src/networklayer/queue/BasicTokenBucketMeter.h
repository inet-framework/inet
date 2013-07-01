//
// Copyright (C) 2013 Kyeong Soo (Joseph) Kim
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


#ifndef __INET_BASICTOKENBUCKETMETER_H
#define __INET_BASICTOKENBUCKETMETER_H

#include "IQoSMeter.h"

/**
 * A meter based on two token buckets one for average and the other for peak rates.
  */
class INET_API BasicDSCPClassifier : public IQoSClassifier
{

    public:
    /**
     * The method should return the result of metering based on two token buckets
     * for the given packet, 0 for conformance and 1 for not.
     */
    virtual void initialize();

    /**
     * The method should return the result of metering based on two token buckets
     * for the given packet, 0 for conformance and 1 for not.
     */
    virtual int meterPacket(cMessage *msg);
};

#endif
