//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TRAILFIGURE_H
#define __INET_TRAILFIGURE_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API TrailFigure : public cGroupFigure
{
  protected:
    int maxCount;
    int fadeCounter;
    bool fadeOut;

  public:
    TrailFigure(int maxCount, bool fadeOut, const char *name = nullptr);

    virtual void addFigure(cFigure *figure) override;
};

} // namespace inet

#endif

