//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RADIOVISUALIZERBASE_H
#define __INET_RADIOVISUALIZERBASE_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/ModuleFilter.h"
#include "inet/visualizer/util/Placement.h"

namespace inet {

namespace visualizer {

class INET_API RadioVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API RadioVisualization {
      public:
        const int radioModuleId = -1;

      public:
        RadioVisualization(const int radioModuleId);
        virtual ~RadioVisualization() {}
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayRadios = false;
    bool displayRadioMode = false;
    bool displayReceptionState = false;
    bool displayTransmissionState = false;
    std::vector<std::string> radioModeImages;
    std::vector<std::string> receptionStateImages;
    std::vector<std::string> transmissionStateImages;
    ModuleFilter radioFilter;
    double width;
    double height;
    Placement placementHint;
    double placementPriority;
    // antennaLobe
    bool antennaLobeNormalize = false;
    bool antennaLobeRelativeLabels = false;
    bool displayAntennaLobes = false;
    bool antennaLobePlaneGlobal = false;
    const char *antennaLobePlane = nullptr;
    const char *antennaLobeMode = nullptr;
    double antennaLobeLogarithmicBase = NaN;
    double antennaLobeLogarithmicScale = NaN;
    double antennaLobeRadius = NaN;
    deg antennaLobeStep = deg(NaN);
    double antennaLobeOpacity = NaN;
    bool antennaLobeLineSmooth = false;
    cFigure::Color antennaLobeLineColor;
    cFigure::LineStyle antennaLobeLineStyle;
    double antennaLobeLineWidth = NaN;
    cFigure::Color antennaLobeFillColor;
    //@}

    std::map<int, const RadioVisualization *> radioVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void refreshDisplay() const override;
    virtual void preDelete(cComponent *root) override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual RadioVisualization *createRadioVisualization(const physicallayer::IRadio *radio) const = 0;
    virtual const RadioVisualization *getRadioVisualization(int radioModuleId);
    virtual void addRadioVisualization(const RadioVisualization *radioVisualization);
    virtual void removeRadioVisualization(const RadioVisualization *radioVisualization);
    virtual void removeAllRadioVisualizations();
    virtual void refreshRadioVisualization(const RadioVisualization *radioVisualization) const = 0;

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, intval_t value, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif

