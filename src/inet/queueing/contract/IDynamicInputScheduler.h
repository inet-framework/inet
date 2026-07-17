//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IDYNAMICINPUTSCHEDULER_H
#define __INET_IDYNAMICINPUTSCHEDULER_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace queueing {

/**
 * Interface for packet schedulers whose set of inputs grows at runtime, so that a dynamic
 * classifier (see ~DynamicPullClassifier) can add an input branch on demand. The classifier
 * creates and connects the new input gate; the scheduler is then told to start considering
 * it via addInput().
 */
class INET_API IDynamicInputScheduler
{
  public:
    virtual ~IDynamicInputScheduler() {}

    /**
     * Registers an input gate that was created and connected at runtime: the scheduler sets
     * up the corresponding provider reference and notifies its downstream collector that a
     * new packet may become available.
     */
    virtual void addInput(cGate *inputGate) = 0;
};

} // namespace queueing
} // namespace inet

#endif
