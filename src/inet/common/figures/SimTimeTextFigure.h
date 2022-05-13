//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIMTIMETEXTFIGURE_H
#define __INET_SIMTIMETEXTFIGURE_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API SimTimeTextFigure : public cTextFigure
{
  protected:
    std::string prefix;

  public:
    using cTextFigure::cTextFigure;
    void refreshDisplay() override;

    void parse(cProperty *property) override;
    const char **getAllowedPropertyKeys() const override;
};

} // namespace inet

#endif

