//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_TRAILFIGURE_H
#define __INET_TRAILFIGURE_H

#include "INETDefs.h"

namespace inet {

class INET_API TrailFigure
#ifdef __CCANVAS_H
: public cGroupFigure
#endif
{
  protected:
    int maxChildCount;

  public:
    TrailFigure(int maxChildCount, const char *name = NULL);

#ifdef __CCANVAS_H
    virtual void addChildFigure(cFigure *figure);
#endif
};

} // namespace inet

#endif // ifndef __INET_TRAILFIGURE_H

