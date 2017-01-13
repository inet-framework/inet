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

#ifndef __INET_MEDIUMOSGVISUALIZER_H
#define __INET_MEDIUMOSGVISUALIZER_H

#include "inet/physicallayer/contract/packetlevel/IRadioFrame.h"
#include "inet/physicallayer/contract/packetlevel/IReceptionDecision.h"
#include "inet/physicallayer/contract/packetlevel/ITransmission.h"
#include "inet/visualizer/base/MediumVisualizerBase.h"
#include "inet/visualizer/scene/NetworkNodeOsgVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API MediumOsgVisualizer : public MediumVisualizerBase
{
#ifdef WITH_OSG

  protected:
    /** @name Parameters */
    //@{
    SignalShape signalShape = SIGNAL_SHAPE_RING;
    const char *signalPlane = nullptr;
    double signalFadingDistance = NaN;
    double signalFadingFactor = NaN;
    double signalWaveLength = NaN;
    double signalWaveAmplitude = NaN;
    double signalWaveFadingAnimationSpeedFactor = NaN;
    osg::Image *transmissionImage = nullptr;
    osg::Image *receptionImage = nullptr;
    //@}

    /** @name Internal state */
    //@{
    NetworkNodeOsgVisualizer *networkNodeVisualizer;
    /**
     * The list of ongoing transmissions.
     */
    std::vector<const ITransmission *> transmissions;
    /**
     * The list of radio osg nodes.
     */
    std::map<const IRadio *, osg::Node *> radioOsgNodes;
    /**
     * The propagating signal osg nodes.
     */
    std::map<const ITransmission *, osg::Node *> signalOsgNodes;
    //@}

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual void setAnimationSpeed() const;

    virtual osg::Node *getRadioOsgNode(const IRadio *radio) const;
    virtual void setRadioOsgNode(const IRadio *radio, osg::Node *node);
    virtual osg::Node *removeRadioOsgNode(const IRadio *radio);

    virtual osg::Node *getSignalOsgNode(const ITransmission *transmission) const;
    virtual void setSignalOsgNode(const ITransmission *transmission, osg::Node *node);
    virtual osg::Node *removeSignalOsgNode(const ITransmission *transmission);

    virtual osg::Node *createSignalNode(const ITransmission *transmission) const;
    virtual osg::Node *createSphereSignalNode(const ITransmission *transmission) const;
    virtual osg::Node *createRingSignalNode(const ITransmission *transmission) const;
    virtual void refreshSphereTransmissionNode(const ITransmission *transmission, osg::Node *node) const;
    virtual void refreshRingTransmissionNode(const ITransmission *transmission, osg::Node *node) const;

    virtual void radioAdded(const IRadio *radio) override;
    virtual void radioRemoved(const IRadio *radio) override;

    virtual void transmissionAdded(const ITransmission *transmission) override;
    virtual void transmissionRemoved(const ITransmission *transmission) override;

    virtual void transmissionStarted(const ITransmission *transmission) override;
    virtual void transmissionEnded(const ITransmission *transmission) override;
    virtual void receptionStarted(const IReception *reception) override;
    virtual void receptionEnded(const IReception *reception) override;

#else // ifdef WITH_OSG

  protected:
    virtual void initialize(int stage) override {}

    virtual void radioAdded(const IRadio *radio) override {}
    virtual void radioRemoved(const IRadio *radio) override {}

    virtual void transmissionAdded(const ITransmission *transmission) override {}
    virtual void transmissionRemoved(const ITransmission *transmission) override {}

    virtual void transmissionStarted(const ITransmission *transmission) override {}
    virtual void transmissionEnded(const ITransmission *transmission) override {}
    virtual void receptionStarted(const IReception *reception) override {}
    virtual void receptionEnded(const IReception *reception) override {}

#endif // ifdef WITH_OSG
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_MEDIUMOSGVISUALIZER_H

