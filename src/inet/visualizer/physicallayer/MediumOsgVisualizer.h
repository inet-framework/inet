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
#include "inet/visualizer/networknode/NetworkNodeOsgVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API MediumOsgVisualizer : public MediumVisualizerBase, public cListener
{
#ifdef WITH_OSG

  protected:
    /** @name Parameters */
    //@{
    double opacityHalfLife = NaN;
    SignalShape signalShape = SIGNAL_SHAPE_RING;
    const char *signalPlane = nullptr;
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
     * The list of ongoing transmission osg nodes.
     */
    std::map<const ITransmission *, osg::Node *> transmissionOsgNodes;
    //@}

    /** @name Timer */
    //@{
    /**
     * The message that is used to update the scene when ongoing communications exist.
     */
    cMessage *signalPropagationUpdateTimer = nullptr;
    //@}

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void refreshDisplay() const override;
    virtual void refreshSphereTransmissionNode(const ITransmission *transmission, osg::Node *node) const;
    virtual void refreshRingTransmissionNode(const ITransmission *transmission, osg::Node *node) const;

    virtual osg::Node *getCachedOsgNode(const IRadio *radio) const;
    virtual void setCachedOsgNode(const IRadio *radio, osg::Node *node);
    virtual osg::Node *removeCachedOsgNode(const IRadio *radio);

    virtual osg::Node *getCachedOsgNode(const ITransmission *transmission) const;
    virtual void setCachedOsgNode(const ITransmission *transmission, osg::Node *node);
    virtual osg::Node *removeCachedOsgNode(const ITransmission *transmission);

    virtual osg::Node *createTransmissionNode(const ITransmission *transmission) const;
    virtual osg::Node *createSphereTransmissionNode(const ITransmission *transmission) const;
    virtual osg::Node *createRingTransmissionNode(const ITransmission *transmission) const;

    virtual void scheduleSignalPropagationUpdateTimer();

    virtual void radioAdded(const IRadio *radio) override;
    virtual void radioRemoved(const IRadio *radio) override;

    virtual void transmissionAdded(const ITransmission *transmission) override;
    virtual void transmissionRemoved(const ITransmission *transmission) override;

    virtual void transmissionStarted(const ITransmission *transmission) override;
    virtual void transmissionEnded(const ITransmission *transmission) override;
    virtual void receptionStarted(const IReception *reception) override;
    virtual void receptionEnded(const IReception *reception) override;

  public:
    virtual ~MediumOsgVisualizer();

#else // ifdef WITH_OSG

  protected:
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

