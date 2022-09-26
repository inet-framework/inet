//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
    class INET_API InfoVisualization {
      public:
        int moduleId = -1;

      public:
        InfoVisualization(int moduleId);
        virtual ~InfoVisualization() {}
    };

    class INET_API DirectiveResolver : public StringFormat::IDirectiveResolver {
      protected:
        const cModule *module = nullptr;

      public:
        DirectiveResolver(const cModule *module) : module(module) {}

        virtual std::string resolveDirective(char directive) const override;
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

    virtual std::string getInfoVisualizationText(cModule *module) const;

  public:
    virtual void preDelete(cComponent *root) override;
};

} // namespace visualizer

} // namespace inet

#endif

