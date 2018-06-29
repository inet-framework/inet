//
// Copyright (C) OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_RAWSOCKET_H
#define __INET_RAWSOCKET_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * TODO
 *
 * See NED file for more details.
 */
class INET_API RawSocket : public cSimpleModule
{
  protected:
    const char *device = nullptr;
    const Protocol *protocol = nullptr;
    int ifindex = -1;

    // statistics
    int numSent = 0;
    int numDropped = 0;

    // state
    int fd = INVALID_SOCKET; // RAW socket ID
    MacAddress myMacAddress;    // for ethernet devices

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    void sendBytes(unsigned char *buf, size_t numBytes, struct sockaddr *from, socklen_t addrlen);

  public:
    virtual ~RawSocket();
};

} // namespace inet

#endif // ifndef __INET_RAWSOCKET_H

