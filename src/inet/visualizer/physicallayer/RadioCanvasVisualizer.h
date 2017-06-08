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

#ifndef __INET_RADIOCANVASVISUALIZER_H
#define __INET_RADIOCANVASVISUALIZER_H

#include "inet/common/figures/IndexedImageFigure.h"
#include "inet/visualizer/base/RadioVisualizerBase.h"
#include "inet/visualizer/scene/NetworkNodeCanvasVisualizer.h"

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

      public:
        RadioCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, IndexedImageFigure *radioModeFigure, IndexedImageFigure *receptionStateFigure, IndexedImageFigure *transmissionStateFigure, const int radioModuleId);
        virtual ~RadioCanvasVisualization();
    };

  protected:
    // parameters
    double zIndex = NaN;
    NetworkNodeCanvasVisualizer *networkNodeVisualizer = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual RadioVisualization *createRadioVisualization(const IRadio *radio) const override;
    virtual void addRadioVisualization(const RadioVisualization *radioVisualization) override;
    virtual void removeRadioVisualization(const RadioVisualization *radioVisualization) override;
    virtual void refreshRadioVisualization(const RadioVisualization *radioVisualization) const override;

    virtual void setImageIndex(IndexedImageFigure *indexedImageFigure, int index) const;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_RADIOCANVASVISUALIZER_H

