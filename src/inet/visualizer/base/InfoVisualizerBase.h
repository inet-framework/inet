//
// Copyright (C) OpenSim Ltd.
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

#ifndef __INET_INFOVISUALIZERBASE_H
#define __INET_INFOVISUALIZERBASE_H

#include "inet/common/StringFormat.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/ModuleFilter.h"
#include "inet/visualizer/util/Placement.h"

namespace inet {

namespace visualizer {

class INET_API InfoVisualizerBase : public VisualizerBase
{
  protected:
    class INET_API InfoVisualization
    {
      public:
        int moduleId = -1;

      public:
        InfoVisualization(int moduleId);
        virtual ~InfoVisualization() {}
    };

    class DirectiveResolver : public StringFormat::IDirectiveResolver {
      protected:
        const cModule *module = nullptr;

      public:
        DirectiveResolver(const cModule *module) : module(module) { }

        virtual const char *resolveDirective(char directive) const override;
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayInfos = false;
    ModuleFilter modules;
    StringFormat format;
    cFigure::Font font;
    cFigure::Color textColor;
    cFigure::Color backgroundColor;
    double opacity = NaN;
    Placement placementHint;
    double placementPriority;
    //@}

    std::vector<const InfoVisualization *> infoVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void refreshDisplay() const override;

    virtual InfoVisualization *createInfoVisualization(cModule *module) const = 0;
    virtual void addInfoVisualization(const InfoVisualization *infoVisualization);
    virtual void addInfoVisualizations();
    virtual void removeInfoVisualization(const InfoVisualization *infoVisualization);
    virtual void refreshInfoVisualization(const InfoVisualization *infoVisualization, const char *info) const = 0;
    virtual void removeAllInfoVisualizations();

    virtual const char *getInfoVisualizationText(cModule *module) const;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_INFOVISUALIZERBASE_H

