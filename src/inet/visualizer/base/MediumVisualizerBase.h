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

#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"
#include "inet/visualizer/base/VisualizerBase.h"

namespace inet {

namespace visualizer {

using namespace inet::physicallayer;

class INET_API MediumVisualizerBase : public VisualizerBase, public cListener
{
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
    IRadioMedium *radioMedium = nullptr;
    bool displaySignals = false;
    simtime_t signalPropagationUpdateInterval = NaN;
    bool displayTransmissions = false;
    bool displayReceptions = false;
    bool displayRadioFrames = false;
    cFigure::Color radioFrameLineColor;
    bool displayCommunicationRanges = false;
    cFigure::Color communicationRangeColor;
    bool displayInterferenceRanges = false;
    cFigure::Color interferenceRangeColor;
    //@}

  protected:
    virtual void initialize(int stage) override;

    virtual simtime_t getNextSignalPropagationUpdateTime(const ITransmission *transmission);

    virtual void radioAdded(const IRadio *radio) = 0;
    virtual void radioRemoved(const IRadio *radio) = 0;

    virtual void transmissionAdded(const ITransmission *transmission) = 0;
    virtual void transmissionRemoved(const ITransmission *transmission) = 0;

    virtual void transmissionStarted(const ITransmission *transmission) = 0;
    virtual void transmissionEnded(const ITransmission *transmission) = 0;
    virtual void receptionStarted(const IReception *reception) = 0;
    virtual void receptionEnded(const IReception *reception) = 0;

  public:
    virtual ~MediumVisualizerBase();
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_MEDIUMVISUALIZERBASE_H

