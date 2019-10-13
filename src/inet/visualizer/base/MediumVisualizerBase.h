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

#ifndef __INET_MEDIUMVISUALIZERBASE_H
#define __INET_MEDIUMVISUALIZERBASE_H

#include "inet/common/packet/PacketFilter.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/ColorSet.h"
#include "inet/visualizer/util/InterfaceFilter.h"
#include "inet/visualizer/util/NetworkNodeFilter.h"
#include "inet/visualizer/util/Placement.h"

#ifdef WITH_RADIO
#include "inet/physicallayer/common/packetlevel/PowerFunctions.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"
#endif // WITH_RADIO

namespace inet {

namespace visualizer {

class INET_API MediumVisualizerBase : public VisualizerBase, public cListener
{
#ifdef WITH_RADIO
  protected:
    enum SignalShape
    {
        SIGNAL_SHAPE_RING,
        SIGNAL_SHAPE_SPHERE,
        SIGNAL_SHAPE_BOTH,
    };

  protected:
    /** @name Parameters */
    //@{
    physicallayer::IRadioMedium *radioMedium = nullptr;
    NetworkNodeFilter networkNodeFilter;
    InterfaceFilter interfaceFilter;
    PacketFilter packetFilter;
    bool displaySignals = false;
    ColorSet signalColorSet;
    double signalPropagationAnimationSpeed = NaN;
    double signalPropagationAnimationTime = NaN;
    double signalPropagationAdditionalTime = NaN;
    double signalTransmissionAnimationSpeed = NaN;
    double signalTransmissionAnimationTime = NaN;
    double signalAnimationSpeedChangeTime = NaN;
    bool displaySignalDepartures = false;
    bool displaySignalArrivals = false;
    Placement signalDeparturePlacementHint;
    Placement signalArrivalPlacementHint;
    double signalDeparturePlacementPriority;
    double signalArrivalPlacementPriority;
    bool displayInterferenceRanges = false;
    cFigure::Color interferenceRangeLineColor;
    cFigure::LineStyle interferenceRangeLineStyle;
    double interferenceRangeLineWidth = NaN;
    bool displayCommunicationRanges = false;
    cFigure::Color communicationRangeLineColor;
    cFigure::LineStyle communicationRangeLineStyle;
    double communicationRangeLineWidth = NaN;
    bool displaySpectrums = false;
    double spectrumFigureWidth = NaN;
    double spectrumFigureHeight = NaN;
    double spectrumFigureInterpolationSize = NaN;
    bool spectrumAutoFrequencyAxis = false;
    Hz spectrumMinFrequency = Hz(NaN);
    Hz spectrumMaxFrequency = Hz(NaN);
    bool spectrumAutoPowerAxis = false;
    WpHz spectrumMinPower = WpHz(NaN);
    WpHz spectrumMaxPower = WpHz(NaN);
    Placement spectrumPlacementHint;
    double spectrumPlacementPriority;
    //@}

    /** @name State */
    //@{
    double defaultSignalPropagationAnimationSpeed = NaN;
    double defaultSignalTransmissionAnimationSpeed = NaN;
    std::map<const physicallayer::ITransmission *, Ptr<const math::IFunction<WpHz, math::Domain<m, m, m, simsec, Hz>>>> receptionPowerFunctions;
    Ptr<math::SumFunction<WpHz, math::Domain<m, m, m, simsec, Hz>>> mediumPowerFunction;
    //@}

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;

    virtual bool isSignalPropagationInProgress(const physicallayer::ITransmission *transmission) const;
    virtual bool isSignalTransmissionInProgress(const physicallayer::ITransmission *transmission) const;

    virtual bool matchesTransmission(const physicallayer::ITransmission *transmission) const;

    virtual void handleRadioAdded(const physicallayer::IRadio *radio) = 0;
    virtual void handleRadioRemoved(const physicallayer::IRadio *radio) = 0;

    virtual void handleSignalAdded(const physicallayer::ITransmission *transmission);
    virtual void handleSignalRemoved(const physicallayer::ITransmission *transmission);

    virtual void handleSignalDepartureStarted(const physicallayer::ITransmission *transmission) = 0;
    virtual void handleSignalDepartureEnded(const physicallayer::ITransmission *transmission) = 0;
    virtual void handleSignalArrivalStarted(const physicallayer::IReception *reception) = 0;
    virtual void handleSignalArrivalEnded(const physicallayer::IReception *reception) = 0;

  public:
    virtual ~MediumVisualizerBase();

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
#endif // WITH_RADIO
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_MEDIUMVISUALIZERBASE_H

