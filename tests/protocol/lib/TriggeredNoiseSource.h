// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_PROTOCOLTEST_TRIGGEREDNOISESOURCE_H
#define __INET_PROTOCOLTEST_TRIGGEREDNOISESOURCE_H

#include "inet/physicallayer/wireless/noise/NoiseSource.h"

namespace inet {
namespace protocoltest {

// This test source uses the production noise transmitter and radio medium.
class INET_API TriggeredNoiseSource : public physicallayer::NoiseSource
{
  protected:
    virtual void scheduleSleepTimer() override;
    virtual void startTransmission() override;
    virtual void endTransmission() override;

  public:
    void trigger();
};

} // namespace protocoltest
} // namespace inet

#endif
