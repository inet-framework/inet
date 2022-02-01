//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INDICATORTEXTFIGURE_H
#define __INET_INDICATORTEXTFIGURE_H

#include "inet/common/INETMath.h"
#include "inet/common/figures/IIndicatorFigure.h"

namespace inet {

class INET_API IndicatorTextFigure : public cTextFigure, public IIndicatorFigure
{
  protected:
    std::string textFormat = "%g";
    double value = NaN;

  protected:
    virtual const char **getAllowedPropertyKeys() const override;
    virtual void parse(cProperty *property) override;
    virtual void refresh();

  public:
    explicit IndicatorTextFigure(const char *name = nullptr) : cTextFigure(name) {}
    virtual const Point getSize() const override { return Point(0, 0); } // TODO
    virtual void setValue(int series, simtime_t timestamp, double value) override;
    virtual const char *getTextFormat() const { return textFormat.c_str(); }
    virtual void setTextFormat(const char *textFormat) { this->textFormat = textFormat; refresh(); }
};

} // namespace inet

#endif

