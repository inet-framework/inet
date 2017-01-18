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
#include "IIndicatorFigure.h"

//TODO namespace inet { -- for the moment commented out, as OMNeT++ 5.0 cannot instantiate a figure from a namespace
using namespace inet;

#if OMNETPP_VERSION >= 0x500

class INET_API IndexedImageFigure : public cGroupFigure, public inet::IIndicatorFigure
{
    std::vector<const char*> images;
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
    std::vector<const char *> getImages() const;
    void setImages(std::vector<const char *> images);

    double getTintAmount() const;
    void setTintAmount(double tintAmount);

    Color getTintColor() const;
    void setTintColor(Color color);

    double getOpacity() const;
    void setOpacity(double opacity);

    Interpolation getInterpolation() const;
    void setInterpolation(Interpolation interpolation);

    const char *getLabel() const;
    void setLabel(const char *text);

    cFigure::Font getLabelFont() const;
    void setLabelFont(cFigure::Font font);

    cFigure::Color getLabelColor() const;
    void setLabelColor(cFigure::Color color);

    Point getLabelOffset() const;
    void setLabelOffset(Point offset);

    Point getSize() const;
    void setSize(Point bounds);

    Point getPos() const;
    void setPos(Point point);

    Anchor getAnchor() const;
    void setAnchor(Anchor anchor);

};

#else

// dummy figure for OMNeT++ 4.x
class INET_API IndexedImageFigure : public cGroupFigure {

};

#endif // omnetpp 5

// } // namespace inet

#endif

