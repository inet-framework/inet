//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/scene/NetworkNodeVsgVisualization.h"

#include <vsg/maths/transform.h>

#include <algorithm>

#include "inet/visualizer/vsg/util/VsgUtils.h"

namespace inet {

namespace visualizer {

NetworkNodeVsgVisualization::NetworkNodeVsgVisualization(cModule *networkNode, bool displayModuleName) :
    NetworkNodeVisualization(networkNode)
{
    cDisplayString& displayString = networkNode->getDisplayString();

    // TODO: render the node's 2D icon (display string "i" tag) as a textured billboard quad, or
    // its 3D model ("osgModel" par), to match the OSG visualizer. For now a colored box marker.
    double markerSize = 10;
    cFigure::Color color("#4080C0");
    // sit the marker ON the ground (bottom at z=0) rather than centered on the ground plane
    auto mainPart = inet::vsg::createBox(Coord(0, 0, markerSize / 2), Coord(markerSize, markerSize, markerSize), color);
    mainPart->setValue("omnetpp.object", (int64_t)(intptr_t)(cObject *)networkNode); // selectable: click -> select module

    auto group = ::vsg::Group::create();
    group->addChild(mainPart);
    size = ::vsg::dvec3(markerSize, 0, markerSize);

    // Shared world anchor (just above the box) for the name label and every annotation. They differ
    // only by an on-screen (screenOffset) amount, so the stack never overlaps regardless of zoom.
    labelPivot = ::vsg::dvec3(0, 0, markerSize + 2);

    if (displayModuleName) {
        // The name label is a camera-facing, screen-constant-size billboard at the anchor (the bottom
        // of the stack); annotations stack above it. characterSize 18 matches OSG (it no longer scales
        // with the world, so the small value that used to compensate for that is gone).
        const double nameSize = 18;
        group->addChild(inet::vsg::createLabel(networkNode->getFullName(), Coord(0, 0, markerSize + 2), cFigure::BLACK, nameSize));
        labelBaseHeight = nameSize + 8;  // reserve the name's on-screen height (+spacing) below annotations
    }

    annotationNode = ::vsg::Group::create();
    group->addChild(annotationNode);
    addChild(group);

    double x = atol(displayString.getTagArg("p", 0));
    double y = atol(displayString.getTagArg("p", 1));
    position = Coord(x, y, 0.0);
    matrix = ::vsg::translate(::vsg::dvec3(x, y, 0.0));
}

void NetworkNodeVsgVisualization::updateAnnotationPositions()
{
    // Stack the annotations in SCREEN space (constant gaps regardless of zoom), above the name label.
    // Each annotation's wrapper is a billboard AutoScaleTransform sharing labelPivot; they differ only
    // by screenOffset.y. (Stacking by world Z instead would collapse to nothing on screen, because the
    // labels are screen-constant size.) This mirrors OSG, which stacks annotations inside the node's
    // autoScaleToScreen AutoTransform.
    const double spacing = 8;
    double stackY = labelBaseHeight;
    for (auto& annotation : annotations) {
        annotation.transform->screenOffset = ::vsg::dvec3(0, stackY, 0);
        double h = std::max(annotation.size.y, 18.0);  // size.y is the content's on-screen height (px-ish)
        stackY += h + spacing;
    }
}

void NetworkNodeVsgVisualization::addAnnotation(::vsg::ref_ptr<::vsg::Node> node, ::vsg::dvec3 size, double priority)
{
    // Wrap the (plain) content in a billboard AutoScaleTransform: it faces the camera, holds a
    // ~constant on-screen size, and carries the screen-space stacking offset. The wrapper is stable
    // even when a visualizer rebuilds its content on refresh (so the stacking survives).
    auto transform = inet::vsg::AutoScaleTransform::create();
    transform->pivot = labelPivot;
    transform->billboard = true;
    transform->refDistance = inet::vsg::LABEL_AUTOSCALE_REF_DISTANCE;
    transform->subgraphRequiresLocalFrustum = false;
    transform->addChild(node);
    annotations.push_back(Annotation(node, transform, size, priority));
    std::stable_sort(annotations.begin(), annotations.end(), [] (const Annotation& a1, const Annotation& a2) {
        return a1.priority < a2.priority;
    });
    annotationNode->addChild(transform);
    updateAnnotationPositions();
}

void NetworkNodeVsgVisualization::removeAnnotation(::vsg::Node *node)
{
    for (auto it = annotations.begin(); it != annotations.end(); it++) {
        if (it->node.get() == node) {
            auto transform = it->transform;
            auto& ch = annotationNode->children;
            ch.erase(std::remove(ch.begin(), ch.end(), transform), ch.end());
            annotations.erase(it);
            break;
        }
    }
    updateAnnotationPositions();
}

void NetworkNodeVsgVisualization::removeAnnotation(int index)
{
    auto it = annotations.begin() + index;
    auto transform = it->transform;
    auto& ch = annotationNode->children;
    ch.erase(std::remove(ch.begin(), ch.end(), transform), ch.end());
    annotations.erase(it);
    updateAnnotationPositions();
}

} // namespace visualizer

} // namespace inet
