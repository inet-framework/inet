//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INDEXEDIMAGEFIGURE_H
#define __INET_INDEXEDIMAGEFIGURE_H

#include "inet/common/INETMath.h"
#include "inet/common/figures/IIndicatorFigure.h"

namespace inet {

class INET_API IndexedImageFigure : public cGroupFigure, public inet::IIndicatorFigure
{
    std::vector<std::string> images;
    cImageFigure *image;
    cTextFigure *labelFigure;

    double value = NaN;

  protected:
    virtual void parse(cProperty *property) override;
    virtual const char **getAllowedPropertyKeys() const override;
    void addChildren();
    void refresh();

  public:
    IndexedImageFigure(const char *name = nullptr);
    virtual ~IndexedImageFigure() {}

    virtual void setValue(int series, simtime_t timestamp, double value) override;

    // getters and setters
    const std::vector<std::string>& getImages() const;
    void setImages(const std::vector<std::string>& images);

    double getTintAmount() const;
    void setTintAmount(double tintAmount);

    const Color& getTintColor() const;
    void setTintColor(const Color& color);

    double getOpacity() const;
    void setOpacity(double opacity);

    Interpolation getInterpolation() const;
    void setInterpolation(Interpolation interpolation);

    const char *getLabel() const;
    void setLabel(const char *text);

    const Font& getLabelFont() const;
    void setLabelFont(const Font& font);

    const Color& getLabelColor() const;
    void setLabelColor(const Color& color);

    const Point getLabelOffset() const;
    void setLabelOffset(const Point& offset);

    virtual const Point getSize() const override;
    void setSize(const Point& bounds);

    const Point& getPos() const;
    void setPos(const Point& point);

    Anchor getAnchor() const;
    void setAnchor(Anchor anchor);
};

} // namespace inet

#endif

