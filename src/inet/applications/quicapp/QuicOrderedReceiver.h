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

#ifndef __INET_QUIC_QUICORDEREDRECEIVER_H_
#define __INET_QUIC_QUICORDEREDRECEIVER_H_

#include <omnetpp.h>
#include "inet/transportlayer/contract/quic/QuicSocket.h"
#include "inet/applications/base/ApplicationBase.h"

using namespace omnetpp;

namespace inet {

class QuicOrderedReceiver : public ApplicationBase
{
protected:
  QuicSocket socket;

protected:
  virtual void handleMessageWhenUp(cMessage *msg) override;

  virtual void handleStartOperation(LifecycleOperation *operation) override;
  virtual void handleStopOperation(LifecycleOperation *operation) override;
  virtual void handleCrashOperation(LifecycleOperation *operation) override;

private:
  uint8_t currentByte = 0;
  void checkData(const Ptr<const Chunk> data);
};

} //namespace

#endif
