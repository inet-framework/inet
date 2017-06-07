//
// Copyright (C) 2016 OpenSim Ltd.
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

#include <sstream>
#include "cPanelFigure.h"

namespace inet {

// NOTE: this cPanelFigure is a copy of the one that will be present in the OMNeT++ 5.2 release
#if OMNETPP_BUILDNUM < 1011

using namespace canvas_stream_ops;

Register_Figure("panel", cPanelFigure);

static const char *PKEY_POS = "pos";
static const char *PKEY_ANCHORPOINT = "anchorPoint";

void cPanelFigure::copy(const cPanelFigure& other)
{
    setPosition(other.getPosition());
    setAnchorPoint(other.getAnchorPoint());
}

cPanelFigure& cPanelFigure::operator=(const cPanelFigure& other)
{
    if (this == &other)
        return *this;
    cFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cPanelFigure::str() const
{
    std::stringstream os;
    os << "at " << getPosition();
    return os.str();
}

void cPanelFigure::parse(cProperty *property)
{
    cFigure::parse(property);
    setPosition(parsePoint(property, PKEY_POS, 0));
    setAnchorPoint(parsePoint(property, PKEY_ANCHORPOINT, 0));
}

const char **cPanelFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_POS, PKEY_ANCHORPOINT, nullptr};
        concatArrays(keys, cFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cPanelFigure::updateParentTransform(Transform& transform)
{
    // replace current transform with an axis-aligned, unscaled (thus also unzoomable)
    // coordinate system, with anchorPoint at getPosition()
    Point origin = transform.applyTo(getPosition());
    Point anchor = getTransform().applyTo(getAnchorPoint());
    transform = Transform().translate(origin.x - anchor.x, origin.y - anchor.y);

    // then apply our own transform in the normal way (like all other figures do)
    transform.rightMultiply(getTransform());
}

#endif

} // namespace inet

