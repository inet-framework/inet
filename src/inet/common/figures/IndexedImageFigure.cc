//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/figures/IndexedImageFigure.h"

#include "inet/common/INETUtils.h"

namespace inet {

Register_Figure("indexedImage", IndexedImageFigure);

static const char *PKEY_IMAGES = "images";
static const char *PKEY_TINT_AMOUNT = "tintAmount";
static const char *PKEY_TINT_COLOR = "tintColor";
static const char *PKEY_OPACITY = "opacity";
static const char *PKEY_INTERPOLATION = "interpolation";
static const char *PKEY_LABEL = "label";
static const char *PKEY_LABEL_FONT = "labelFont";
static const char *PKEY_LABEL_COLOR = "labelColor";
static const char *PKEY_LABEL_OFFSET = "labelOffset";
static const char *PKEY_INITIAL_VALUE = "initialValue";
static const char *PKEY_POS = "pos";
static const char *PKEY_SIZE = "size";
static const char *PKEY_ANCHOR = "anchor";
static const char *PKEY_BOUNDS = "bounds";

IndexedImageFigure::IndexedImageFigure(const char *name) : cGroupFigure(name)
{
    addChildren();
}

const std::vector<std::string>& IndexedImageFigure::getImages() const
{
    return images;
}

void IndexedImageFigure::setImages(const std::vector<std::string>& images)
{
    ASSERT(images.size());
    this->images = images;
    refresh();
}

double IndexedImageFigure::getTintAmount() const
{
    return image->getTintAmount();
}

void IndexedImageFigure::setTintAmount(double tintAmount)
{
    image->setTintAmount(tintAmount);
}

const cFigure::Color& IndexedImageFigure::getTintColor() const
{
    return image->getTintColor();
}

void IndexedImageFigure::setTintColor(const Color& color)
{
    image->setTintColor(color);
}

double IndexedImageFigure::getOpacity() const
{
    return image->getOpacity();
}

void IndexedImageFigure::setOpacity(double opacity)
{
    image->setOpacity(opacity);
}

cFigure::Interpolation IndexedImageFigure::getInterpolation() const
{
    return image->getInterpolation();
}

void IndexedImageFigure::setInterpolation(Interpolation interpolation)
{
    image->setInterpolation(interpolation);
}

const char *IndexedImageFigure::getLabel() const
{
    return labelFigure->getText();
}

void IndexedImageFigure::setLabel(const char *text)
{
    labelFigure->setText(text);
}

const cFigure::Font& IndexedImageFigure::getLabelFont() const
{
    return labelFigure->getFont();
}

void IndexedImageFigure::setLabelFont(const Font& font)
{
    labelFigure->setFont(font);
}

const cFigure::Color& IndexedImageFigure::getLabelColor() const
{
    return labelFigure->getColor();
}

void IndexedImageFigure::setLabelColor(const Color& color)
{
    labelFigure->setColor(color);
}

const cFigure::Point IndexedImageFigure::getLabelOffset() const
{
    return labelFigure->getPosition() - image->getPosition();
}

void IndexedImageFigure::setLabelOffset(const Point& offset)
{
    labelFigure->setPosition(image->getPosition() + offset);
}

const cFigure::Point IndexedImageFigure::getSize() const
{
    return Point(image->getWidth(), image->getHeight());
}

void IndexedImageFigure::setSize(const Point& bounds)
{
    image->setWidth(bounds.x);
    image->setHeight(bounds.y);
}

const cFigure::Point& IndexedImageFigure::getPos() const
{
    return image->getPosition();
}

void IndexedImageFigure::setPos(const Point& pos)
{
    Point offset = getLabelOffset();
    image->setPosition(pos);
    setLabelOffset(offset);
}

cFigure::Anchor IndexedImageFigure::getAnchor() const
{
    return image->getAnchor();
}

void IndexedImageFigure::setAnchor(Anchor anchor)
{
    image->setAnchor(anchor);
}

void IndexedImageFigure::parse(cProperty *property)
{
    ASSERT(property->getNumValues(PKEY_IMAGES));
    cGroupFigure::parse(property);

    const char *s;

    std::vector<std::string> names;
    for (int i = 0; i < property->getNumValues(PKEY_IMAGES); ++i)
        names.push_back(property->getValue(PKEY_IMAGES, i));
    setImages(names);

    if (property->containsKey(PKEY_BOUNDS)) {
        if (property->containsKey(PKEY_POS) || property->containsKey(PKEY_SIZE) || property->containsKey(PKEY_ANCHOR))
            throw cRuntimeError("%s, %s and %s are not allowed when %s is present", PKEY_POS, PKEY_SIZE, PKEY_ANCHOR, PKEY_BOUNDS);
        if (property->getNumValues(PKEY_BOUNDS) != 4)
            throw cRuntimeError("%s: x, y, width, height expected", PKEY_BOUNDS);
        Point p = parsePoint(property, PKEY_BOUNDS, 0);
        Point size = parsePoint(property, PKEY_BOUNDS, 2);
        setPos(p);
        setSize(size);
        setAnchor(ANCHOR_NW);
    }
    else {
        setPos(parsePoint(property, PKEY_POS, 0));
        if ((s = property->getValue(PKEY_ANCHOR)) != nullptr)
            setAnchor(parseAnchor(s));
        if ((s = property->getValue(PKEY_SIZE)) != nullptr)
            setSize(parsePoint(property, PKEY_SIZE, 0));
    }
    if ((s = property->getValue(PKEY_TINT_AMOUNT)) != nullptr)
        setTintAmount(utils::atod(s));
    if ((s = property->getValue(PKEY_TINT_COLOR)) != nullptr)
        setTintColor(parseColor(s));
    if ((s = property->getValue(PKEY_OPACITY)) != nullptr)
        setOpacity(atof(s));
    if ((s = property->getValue(PKEY_INTERPOLATION)) != nullptr)
        setInterpolation(parseInterpolation(s));
    if ((s = property->getValue(PKEY_LABEL)) != nullptr)
        setLabel(s);
    if ((s = property->getValue(PKEY_LABEL_FONT)) != nullptr)
        setLabelFont(parseFont(s));
    if ((s = property->getValue(PKEY_LABEL_COLOR)) != nullptr)
        setLabelColor(parseColor(s));
    if (property->containsKey(PKEY_LABEL_OFFSET))
        setLabelOffset(parsePoint(property, PKEY_LABEL_OFFSET, 0));
    if ((s = property->getValue(PKEY_INITIAL_VALUE)) != nullptr)
        setValue(0, simTime(), utils::atod(s));
}

const char **IndexedImageFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = {
            PKEY_IMAGES, PKEY_TINT_AMOUNT, PKEY_TINT_COLOR, PKEY_OPACITY,
            PKEY_INTERPOLATION, PKEY_LABEL, PKEY_LABEL_FONT, PKEY_LABEL_COLOR,
            PKEY_LABEL_OFFSET, PKEY_INITIAL_VALUE, PKEY_POS, PKEY_SIZE, PKEY_ANCHOR,
            PKEY_BOUNDS, nullptr
        };
        concatArrays(keys, cGroupFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void IndexedImageFigure::addChildren()
{
    labelFigure = new cTextFigure("label");
    image = new cImageFigure("image");

    labelFigure->setAnchor(ANCHOR_N);

    addFigure(image);
    addFigure(labelFigure);
}

void IndexedImageFigure::setValue(int series, simtime_t timestamp, double newValue)
{
    ASSERT(series == 0);
    // Note: we currently ignore timestamp
    if (value != newValue) {
        value = newValue;
        refresh();
    }
}

void IndexedImageFigure::refresh()
{
    if (std::isnan(value)) {
        image->setVisible(false);
        return;
    }

    image->setVisible(true);
    int newValue = (int)value % images.size();
    image->setImageName(images[newValue].c_str());
}

} // namespace inet

