//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.scene;

import inet.visualizer.base.SceneVisualizerBase;
import inet.visualizer.contract.ISceneVisualizer;

//
// Visualizes the simulation scene on a 2D canvas. Provides the coordinate system
// and projection that serves as the foundation for all other canvas visualizers.
//
// The visualization includes:
// - 2D orthographic projection of the 3D coordinate system
// - Configurable view angle, scale, and translation
// - Optional coordinate system axes display
// - Support for displaying the active configuration description
//
// Particularly useful for establishing a consistent visual reference frame
// across all visualizers and controlling the overall view of the simulation.
//
// @see ~SceneOsgVisualizer, ~SceneVisualizer, ~SceneVisualizerBase, ~ISceneVisualizer
//
simple SceneCanvasVisualizer extends SceneVisualizerBase like ISceneVisualizer
{
    parameters:
        double zIndex = default(0); // Determines the drawing order of figures relative to other visualizers
        string viewAngle @mutable = default("x right y down z out"); // View angle is either
                                                                     //   - a valid combination of the words: x, y, z, left/right, up/down, in/out or
                                                                     //   - one of isometric, isometricN (where 0 <= N < 48)
                                                                     //   - 3 rotation angles and leftHanded/rightHanded
        string viewScale @mutable = default("1 1"); // The 2D scale is applied after the view projection
        string viewTranslation @mutable = default("0 0"); // The 2D translation is applied after the view projection
        string descriptionFigure = default(""); // If not empty, it should be the name of a text figure; it will display the active configuration's description
        @class(SceneCanvasVisualizer);
}
