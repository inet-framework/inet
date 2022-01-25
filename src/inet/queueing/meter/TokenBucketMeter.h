//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_TOKENBUCKETMETER_H
#define __INET_TOKENBUCKETMETER_H

#include "inet/queueing/base/PacketMeterBase.h"
#include "inet/queueing/base/TokenBucketMeterMixin.h"
#include "inet/queueing/base/TokenBucketMixin.h"

namespace inet {
namespace queueing {

class INET_API TokenBucketMeter : public TokenBucketMeterMixin<TokenBucketMixin<PacketMeterBase>>
{
  protected:
    const char *label = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual void meterPacket(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif

