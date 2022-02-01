//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INOISE_H
#define __INET_INOISE_H

#include "inet/common/IPrintableObject.h"

namespace inet {

namespace physicallayer {

/**
 * This interface represents a meaningless radio signal.
 */
class INET_API INoise : public IPrintableObject
{
  public:
    virtual const simtime_t getStartTime() const = 0;
    virtual const simtime_t getEndTime() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

