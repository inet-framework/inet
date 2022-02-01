//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INDICATORLABELFIGURE_H
#define __INET_INDICATORLABELFIGURE_H

#include "inet/common/INETMath.h"
#include "inet/common/figures/IIndicatorFigure.h"

namespace inet {

class INET_API IndicatorLabelFigure : public cLabelFigure, public IIndicatorFigure
{
  protected:
    std::string textFormat = "%g";
    double value = NaN;

  protected:
    virtual const char **getAllowedPropertyKeys() const override;
    virtual void parse(cProperty *property) override;
    virtual void refresh();

  public:
    explicit IndicatorLabelFigure(const char *name = nullptr) : cLabelFigure(name) {}
    virtual const Point getSize() const override { return getBounds().getSize(); }
    virtual void setValue(int series, simtime_t timestamp, double value) override;
    virtual const char *getTextFormat() const { return textFormat.c_str(); }
    virtual void setTextFormat(const char *textFormat) { this->textFormat = textFormat; refresh(); }
};

} // namespace inet

#endif

