//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MEDIUMOSGVISUALIZER_H
#define __INET_MEDIUMOSGVISUALIZER_H

#include "inet/visualizer/base/MediumVisualizerBase.h"
#include "inet/visualizer/osg/scene/NetworkNodeOsgVisualizer.h"

#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReceptionDecision.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmission.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IWirelessSignal.h"
#endif // INET_WITH_PHYSICALLAYERWIRELESSCOMMON

namespace inet {

namespace visualizer {

class INET_API MediumOsgVisualizer : public MediumVisualizerBase
{
#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON

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
    osg::Image *signalDepartureImage = nullptr;
    osg::Image *signalArrivalImage = nullptr;
    //@}

    /** @name Internal state */
    //@{
    ModuleRefByPar<NetworkNodeOsgVisualizer> networkNodeVisualizer;
    /**
     * The list of ongoing transmissions.
     */
    std::vector<const physicallayer::ITransmission *> transmissions;
    /**
     * The map of radio osg nodes.
     */
    std::map<int, osg::Node *> radioOsgNodes;
    /**
     * The map signal osg nodes for ongoing transmissions.
     */
    std::map<int, osg::Node *> signalOsgNodes;
    //@}

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual void setAnimationSpeed() const;

    virtual osg::Node *getRadioOsgNode(const physicallayer::IRadio *radio) const;
    virtual void setRadioOsgNode(const physicallayer::IRadio *radio, osg::Node *node);
    virtual osg::Node *removeRadioOsgNode(const physicallayer::IRadio *radio);

    virtual osg::Node *getSignalOsgNode(const physicallayer::ITransmission *transmission) const;
    virtual void setSignalOsgNode(const physicallayer::ITransmission *transmission, osg::Node *node);
    virtual osg::Node *removeSignalOsgNode(const physicallayer::ITransmission *transmission);

    virtual osg::Node *createSignalNode(const physicallayer::ITransmission *transmission) const;
    virtual osg::Node *createSphereSignalNode(const physicallayer::ITransmission *transmission) const;
    virtual osg::Node *createRingSignalNode(const physicallayer::ITransmission *transmission) const;
    virtual void refreshSphereTransmissionNode(const physicallayer::ITransmission *transmission, osg::Node *node) const;
    virtual void refreshRingTransmissionNode(const physicallayer::ITransmission *transmission, osg::Node *node) const;

    virtual void handleRadioAdded(const physicallayer::IRadio *radio) override;
    virtual void handleRadioRemoved(const physicallayer::IRadio *radio) override;

    virtual void handleSignalAdded(const physicallayer::ITransmission *transmission) override;
    virtual void handleSignalRemoved(const physicallayer::ITransmission *transmission) override;

    virtual void handleSignalDepartureStarted(const physicallayer::ITransmission *transmission) override;
    virtual void handleSignalDepartureEnded(const physicallayer::ITransmission *transmission) override;
    virtual void handleSignalArrivalStarted(const physicallayer::IReception *reception) override;
    virtual void handleSignalArrivalEnded(const physicallayer::IReception *reception) override;

#endif // ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
};

} // namespace visualizer

} // namespace inet

#endif

