//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_VISUALIZERBASE_H
#define __INET_VISUALIZERBASE_H

#include <functional>

#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/common/packet/chunk/Chunk.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

namespace visualizer {

class INET_API VisualizerBase : public cSimpleModule
{
  protected:
    cModule *visualizationTargetModule = nullptr;
    cModule *visualizationSubjectModule = nullptr;
    const char *tags = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual Coord getPosition(const cModule *networkNode) const;
    virtual Coord getContactPosition(const cModule *networkNode, const Coord& fromPosition, const char *contactMode, double contactSpacing) const;
    virtual Quaternion getOrientation(const cModule *networkNode) const;

    virtual void mapChunks(const Ptr<const Chunk>& chunk, const std::function<void(const Ptr<const Chunk>&, int)>& thunk) const;
};

} // namespace visualizer

} // namespace inet

#endif

