//
// Copyright (C) 2016 OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_INDEXEDIMAGEFIGURE_H
#define __INET_INDEXEDIMAGEFIGURE_H

#include "inet/common/INETDefs.h"
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
    virtual ~IndexedImageFigure() {};

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

