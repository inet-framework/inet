//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MEDIUMVSGVISUALIZER_H
#define __INET_MEDIUMVSGVISUALIZER_H

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>

#include "inet/visualizer/base/MediumVisualizerBase.h"
#include "inet/visualizer/vsg/scene/NetworkNodeVsgVisualizer.h"

#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReceptionDecision.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmission.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IWirelessSignal.h"
#endif // INET_WITH_PHYSICALLAYERWIRELESSCOMMON

namespace inet {

namespace visualizer {

class INET_API MediumVsgVisualizer : public MediumVisualizerBase
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
    bool signalWaveShader = false; // experimental: draw the ring with the GLSL wave shader (smooth flowing ripple) instead of baked per-vertex alpha
    bool rangeSphere = false;      // draw communication/interference range indicators as 3D wireframe spheres instead of flat circles
    //@}

    /** @name Internal state */
    //@{
    ModuleRefByPar<NetworkNodeVsgVisualizer> networkNodeVisualizer;
    /**
     * The list of ongoing transmissions.
     */
    std::vector<const physicallayer::ITransmission *> transmissions;
    /**
     * Maps radio id -> VSG annotation Group attached to the network-node visualization.
     * The group holds sub-nodes for departure icon, arrival icon, interference/communication
     * range circles (in that order, same as the OSG twin).
     */
    std::map<int, ::vsg::ref_ptr<::vsg::Group>> radioVsgNodes;
    /**
     * Maps transmission id -> top-level VSG node added to the simulation scene.
     * For SIGNAL_SHAPE_RING: a MatrixTransform (position) holding the ring Group.
     * For SIGNAL_SHAPE_SPHERE: a Group holding two MatrixTransforms (start/end sphere).
     * For SIGNAL_SHAPE_BOTH: a Group holding one ring node and one sphere node.
     */
    std::map<int, ::vsg::ref_ptr<::vsg::Node>> signalVsgNodes;
    //@}

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual void setAnimationSpeed() const;

    // --- per-radio node management ---
    virtual ::vsg::ref_ptr<::vsg::Group> getRadioVsgNode(const physicallayer::IRadio *radio) const;
    virtual void setRadioVsgNode(const physicallayer::IRadio *radio, ::vsg::ref_ptr<::vsg::Group> node);
    virtual ::vsg::ref_ptr<::vsg::Group> removeRadioVsgNode(const physicallayer::IRadio *radio);

    // --- per-transmission node management ---
    virtual ::vsg::ref_ptr<::vsg::Node> getSignalVsgNode(const physicallayer::ITransmission *transmission) const;
    virtual void setSignalVsgNode(const physicallayer::ITransmission *transmission, ::vsg::ref_ptr<::vsg::Node> node);
    virtual ::vsg::ref_ptr<::vsg::Node> removeSignalVsgNode(const physicallayer::ITransmission *transmission);

    // --- signal node creation / refresh ---
    virtual ::vsg::ref_ptr<::vsg::Node> createSignalNode(const physicallayer::ITransmission *transmission) const;
    virtual ::vsg::ref_ptr<::vsg::Node> createRingSignalNode(const physicallayer::ITransmission *transmission) const;
    virtual ::vsg::ref_ptr<::vsg::Node> createSphereSignalNode(const physicallayer::ITransmission *transmission) const;

    virtual void refreshRingTransmissionNode(const physicallayer::ITransmission *transmission, ::vsg::Node *node) const;
    virtual void refreshSphereTransmissionNode(const physicallayer::ITransmission *transmission, ::vsg::Node *node) const;

    // --- base-class virtuals ---
    virtual void handleRadioAdded(const physicallayer::IRadio *radio) override;
    virtual void handleRadioRemoved(const physicallayer::IRadio *radio) override;

    virtual void handleSignalAdded(const physicallayer::ITransmission *transmission) override;
    virtual void handleSignalRemoved(const physicallayer::ITransmission *transmission) override;

    virtual void handleSignalDepartureStarted(const physicallayer::ITransmission *transmission) override;
    virtual void handleSignalDepartureEnded(const physicallayer::ITransmission *transmission) override;
    virtual void handleSignalArrivalStarted(const physicallayer::IReception *reception) override;
    virtual void handleSignalArrivalEnded(const physicallayer::IReception *reception) override;

#endif // INET_WITH_PHYSICALLAYERWIRELESSCOMMON
};

} // namespace visualizer

} // namespace inet

#endif
