/*
 * Copyright (C) 2009 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __INET_HOSTAUTOCONFIGURATOR_H
#define __INET_HOSTAUTOCONFIGURATOR_H

#include <omnetpp.h>
#include "inet/common/INETDefs.h"

namespace inet {

/**
 * HostAutoConfigurator automatically assigns IP addresses and sets up routing table.
 *
 * @author Christoph Sommer
 */
class INET_API HostAutoConfigurator : public cSimpleModule
{
  public:
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    virtual void handleMessage(cMessage *msg) override;

  protected:
    void setupNetworkLayer();
};

} // namespace inet

#endif // ifndef __INET_HOSTAUTOCONFIGURATOR_H

