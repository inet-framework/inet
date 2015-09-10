//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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

#ifndef __INET_THRESHOLDDROPPER_H
#define __INET_THRESHOLDDROPPER_H

#include "inet/common/INETDefs.h"
#include "inet/common/queue/AlgorithmicDropperBase.h"

namespace inet {

/**
 * Drops packets above threshold.
 * Resources can be shared amongst multiple queues.
 */
class INET_API ThresholdDropper : public AlgorithmicDropperBase
{
  protected:
    int frameCapacity;
    int byteCapacity;

  protected:
    virtual void initialize() override;
    virtual bool shouldDrop(cPacket *packet) override;
};

} // namespace inet

#endif // ifndef __INET_THRESHOLDDROPPER_H

