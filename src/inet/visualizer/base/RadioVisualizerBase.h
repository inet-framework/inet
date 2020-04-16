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

#ifndef __INET_RADIOVISUALIZERBASE_H
#define __INET_RADIOVISUALIZERBASE_H

#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/Placement.h"
#include "inet/visualizer/util/ModuleFilter.h"

namespace inet {

namespace visualizer {

class INET_API RadioVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API RadioVisualization
    {
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

    virtual void subscribe();
    virtual void unsubscribe();

    virtual RadioVisualization *createRadioVisualization(const physicallayer::IRadio *radio) const = 0;
    virtual const RadioVisualization *getRadioVisualization(int radioModuleId);
    virtual void addRadioVisualization(const RadioVisualization *radioVisualization);
    virtual void removeRadioVisualization(const RadioVisualization *radioVisualization);
    virtual void refreshRadioVisualization(const RadioVisualization *radioVisualization) const = 0;

  public:
    virtual ~RadioVisualizerBase();

    virtual void receiveSignal(cComponent *source, simsignal_t signal, intval_t value, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_RADIOVISUALIZERBASE_H

