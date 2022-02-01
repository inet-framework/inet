//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TESTRADIO_H_
#define __INET_TESTRADIO_H_

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API RadioTest : public cSimpleModule
{
  public:
    RadioTest() {}

  protected:
    virtual void initialize(int stage);
};

} // namespace inet

#endif
