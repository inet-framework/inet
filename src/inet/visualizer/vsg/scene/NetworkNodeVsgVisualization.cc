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
    auto group = ::vsg::Group::create();
    double representationTop;   // on-screen height (above labelPivot) taken by the node representation

    // --- representation: the node's 2D icon (display string "i" tag) as a camera-facing, constant-size
    //     textured billboard (matching the OSG visualizer); fall back to a colored box marker if there
    //     is no icon or it cannot be loaded. (TODO: 3D model from the "osgModel"/"model" parameter.)
    ::vsg::ref_ptr<::vsg::Data> iconImage;
    const char *iconName = displayString.getTagArg("i", 0);
    if (iconName && *iconName) {
        try { iconImage = inet::vsg::createImage(inet::vsg::resolveImageResource(iconName, networkNode).c_str()); }
        catch (const std::exception&) { iconImage = nullptr; }  // missing icon -> box fallback
    }
    if (iconImage) {
        const double iconSize = 40;            // nominal on-screen icon size (label units)
        labelPivot = ::vsg::dvec3(0, 0, 0);    // anchor the whole stack at the node base; stack up in screen space
        auto iconBillboard = inet::vsg::AutoScaleTransform::create();
        iconBillboard->pivot = labelPivot;
        iconBillboard->billboard = true;
        iconBillboard->refDistance = inet::vsg::LABEL_AUTOSCALE_REF_DISTANCE;
        iconBillboard->subgraphRequiresLocalFrustum = false;
        iconBillboard->screenOffset = ::vsg::dvec3(0, iconSize / 2, 0);  // icon bottom sits at the node base
        iconBillboard->setValue("omnetpp.object", (int64_t)(intptr_t)(cObject *)networkNode); // selectable
        iconBillboard->addChild(inet::vsg::createTexturedQuad(iconImage, iconSize));
        group->addChild(iconBillboard);
        size = ::vsg::dvec3(iconSize, 0, iconSize);
        representationTop = iconSize + 6;
    }
    else {
        const double markerSize = 10;
        cFigure::Color color("#4080C0");
        // sit the marker ON the ground (bottom at z=0) rather than centered on the ground plane
        auto box = inet::vsg::createBox(Coord(0, 0, markerSize / 2), Coord(markerSize, markerSize, markerSize), color);
        box->setValue("omnetpp.object", (int64_t)(intptr_t)(cObject *)networkNode); // selectable: click -> select module
        group->addChild(box);
        size = ::vsg::dvec3(markerSize, 0, markerSize);
        labelPivot = ::vsg::dvec3(0, 0, markerSize + 2);  // labels anchor at the box top (world)
        representationTop = 0;
    }

    // The name label and all annotations are camera-facing, screen-constant-size billboards sharing
    // labelPivot, separated only by an on-screen (screenOffset) amount, so the stack never overlaps
    // regardless of zoom. The name is the bottom of the stack (just above the representation).
    if (displayModuleName) {
        const double nameSize = 18;
        auto nameBillboard = inet::vsg::AutoScaleTransform::create();
        nameBillboard->pivot = labelPivot;
        nameBillboard->billboard = true;
        nameBillboard->refDistance = inet::vsg::LABEL_AUTOSCALE_REF_DISTANCE;
        nameBillboard->subgraphRequiresLocalFrustum = false;
        nameBillboard->screenOffset = ::vsg::dvec3(0, representationTop, 0);
        nameBillboard->addChild(inet::vsg::createText(networkNode->getFullName(), Coord::ZERO, cFigure::BLACK, nameSize));
        group->addChild(nameBillboard);
        labelBaseHeight = representationTop + nameSize + 8;  // annotations stack above the name
    }
    else
        labelBaseHeight = representationTop;

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
