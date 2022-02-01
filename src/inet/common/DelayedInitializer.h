//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DELAYEDINITIALIZER_H
#define __INET_DELAYEDINITIALIZER_H

#include <functional>

#include "inet/common/INETDefs.h"

namespace inet {

// this is a singleton
template<typename T> class INET_API DelayedInitializer
{
  private:
    mutable T *t = nullptr;
    bool constructed;
    const std::function<T *()> initializer;

  public:
    DelayedInitializer(const std::function<T *()> initializer) : constructed(true), initializer(initializer) {}
    ~DelayedInitializer() { delete t; }

    const T *operator&() const {
        if (!constructed) {
            fprintf(stderr, "Cannot dereference DelayedInitializer<%s> before it is completely constructed\n", typeid(t).name());
            abort();
        }
        if (t == nullptr)
            t = initializer();
        return t;
    }
};

} /* namespace inet */

#endif

