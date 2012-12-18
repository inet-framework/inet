//
// Copyright (C) 2012 OpenSim Ltd.
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

#ifndef __INET_INTERNETCLOUD_TAGGERCHANNEL_H
#define __INET_INTERNETCLOUD_TAGGERCHANNEL_H


#include "INETDefs.h"

class InterfaceEntry;

/**
 * Implementation of InternetCloudTaggerChannel. See NED file for details.
 */
class InternetCloudTaggerChannel : public cIdealChannel
{
    InterfaceEntry *ie;

  public:
    /**
     * Constructor. This is only for internal purposes, and should not
     * be used when creating channels dynamically; use the create()
     * factory method instead.
     */
    explicit InternetCloudTaggerChannel(const char *name=NULL)
            : cIdealChannel(name), ie(NULL) {}

    /**
     * Destructor.
     */
    virtual ~InternetCloudTaggerChannel() {}

    /**
     * The implementation of this method adds incoming interfaceId to msg as par("incomingInterfaceID").
     */
    virtual void processMessage(cMessage *msg, simtime_t t, result_t& result);

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const { return 2; }
};

#endif // __INET_INTERNETCLOUD_TAGGERCHANNEL_H
