//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.scene;

import inet.visualizer.base.SceneVisualizerBase;
import inet.visualizer.contract.ISceneVisualizer;

//
// This module visualizes the scene on a 2D canvas. It provides the 3D coordinate
// system's 2D orthographic projection for other visualizers. This is used to get
// a consistent visualization accross all visualizers. It can also display the
// coordinate system axes.
//
// @see ~SceneOsgVisualizer, ~SceneVisualizer, ~SceneVisualizerBase, ~ISceneVisualizer
//
simple SceneCanvasVisualizer extends SceneVisualizerBase like ISceneVisualizer
{
    parameters:
        double zIndex = default(0); // determines the drawing order of figures relative to other visualizers
        string viewAngle @mutable = default("x right y down z out"); // view angle is either ()
                                                                     //   - a valid combination of the words: x, y, z, left/right, up/down, in/out or
                                                                     //   - one of isometric, isometricN (where 0 <= N < 48)
                                                                     //   - 3 rotation angles and leftHanded/rightHanded
        string viewScale @mutable = default("1 1"); // the 2D scale is applied after the view projection
        string viewTranslation @mutable = default("0 0"); // the 2D translation is applied after the view projection
        string descriptionFigure = default(""); // if not empty, it should be the name of a text figure; it will display the active configuration's description
        @class(SceneCanvasVisualizer);
}

