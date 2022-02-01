//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RADIOCANVASVISUALIZER_H
#define __INET_RADIOCANVASVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/figures/IndexedImageFigure.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IAntenna.h"
#include "inet/visualizer/base/RadioVisualizerBase.h"
#include "inet/visualizer/canvas/scene/NetworkNodeCanvasVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API RadioCanvasVisualizer : public RadioVisualizerBase
{
  protected:
    class INET_API RadioCanvasVisualization : public RadioVisualization {
      public:
        NetworkNodeCanvasVisualization *networkNodeVisualization = nullptr;
        IndexedImageFigure *radioModeFigure = nullptr;
        IndexedImageFigure *receptionStateFigure = nullptr;
        IndexedImageFigure *transmissionStateFigure = nullptr;
        cPolygonFigure *antennaLobeFigure = nullptr;
        cOvalFigure *antennaLobeUnitGainFigure = nullptr;
        cOvalFigure *antennaLobeMaxGainFigure = nullptr;

      public:
        RadioCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, IndexedImageFigure *radioModeFigure, IndexedImageFigure *receptionStateFigure, IndexedImageFigure *transmissionStateFigure, cPolygonFigure *antennaLobeFigure, cOvalFigure *antennaLobeUnitGainFigure, cOvalFigure *antennaLobeMaxGainFigure, const int radioModuleId);
        virtual ~RadioCanvasVisualization();
    };

  protected:
    // parameters
    double zIndex = NaN;
    const CanvasProjection *canvasProjection = nullptr;
    ModuleRefByPar<NetworkNodeCanvasVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual RadioVisualization *createRadioVisualization(const physicallayer::IRadio *radio) const override;
    virtual void addRadioVisualization(const RadioVisualization *radioVisualization) override;
    virtual void removeRadioVisualization(const RadioVisualization *radioVisualization) override;
    virtual void refreshRadioVisualization(const RadioVisualization *radioVisualization) const override;
    virtual void refreshAntennaLobe(const inet::physicallayer::IAntenna *antenna, cPolygonFigure *antennaLobeFigure, cOvalFigure *antennaLobeUnitGainFigure, cOvalFigure *antennaLobeMaxGainFigure) const;

    virtual void setImageIndex(IndexedImageFigure *indexedImageFigure, int index) const;
    virtual double getGainRadius(double gain, double maxGain) const;
};

} // namespace visualizer

} // namespace inet

#endif

