//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/scene/NetworkNodeOsgVisualization.h"

#include <omnetpp/osgutil.h>

#include <osg/AutoTransform>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgText/Font>
#include <osgText/Text>

#include <algorithm>

#include "inet/visualizer/osg/util/OsgUtils.h"

namespace inet {

namespace visualizer {

NetworkNodeOsgVisualization::Annotation::Annotation(osg::Node *node, osg::Vec3d size, double priority) :
    node(node),
    size(size),
    priority(priority)
{
}

NetworkNodeOsgVisualization::NetworkNodeOsgVisualization(cModule *networkNode, bool displayModuleName) :
    NetworkNodeVisualizerBase::NetworkNodeVisualization(networkNode)
{
    double spacing = 4;
    osgText::Text *label = nullptr;
    if (displayModuleName) {
        auto font = osgText::Font::getDefaultFont();
        label = new osgText::Text();
        label->setCharacterSize(18);
        label->setBoundingBoxColor(osg::Vec4(1.0, 1.0, 1.0, 1.0));
        label->setBoundingBoxMargin(spacing);
        label->setColor(osg::Vec4(0.0, 0.0, 0.0, 1.0));
        label->setAlignment(osgText::Text::CENTER_BOTTOM);
        label->setText(networkNode->getFullName());
        label->setDrawMode(osgText::Text::FILLEDBOUNDINGBOX | osgText::Text::TEXT);
        label->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
        for (auto texture : font->getGlyphTextureList()) {
            texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
            texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
        }
    }
    osg::Node *osgNode = nullptr;
    cDisplayString& displayString = networkNode->getDisplayString();
    if (networkNode->hasPar("osgModel") && strlen(networkNode->par("osgModel")) != 0) {
        auto osgModelPath = getOsgModelPath(networkNode);
        auto osgModel = osgDB::readNodeFile(osgModelPath.c_str());
        if (osgModel == nullptr)
            throw cRuntimeError("Visual representation osg model '%s' not found", networkNode->par("osgModel").stringValue());
        const char *osgModelColor = networkNode->par("osgModelColor");
        if (*osgModelColor != '\0') {
            auto material = new osg::Material();
            auto color = cFigure::Color(osgModelColor);
            osg::Vec4 colorVec((double)color.red / 255.0, (double)color.green / 255.0, (double)color.blue / 255.0, 1.0);
            material->setAmbient(osg::Material::FRONT_AND_BACK, colorVec);
            material->setDiffuse(osg::Material::FRONT_AND_BACK, colorVec);
            material->setAlpha(osg::Material::FRONT_AND_BACK, 1.0);
            osgModel->getOrCreateStateSet()->setAttribute(material);
        }
        auto group = new osg::Group();
        group->addChild(osgModel);
        if (displayModuleName) {
            label->setPosition(osg::Vec3(0.0, spacing, 0.0));
            auto geode = new osg::Geode();
            geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
            geode->addDrawable(label);
            auto boundingSphere = osgModel->getBound();
            size = osg::Vec3d(0, 18, 0);
            annotationNode = new osg::Group();
            annotationNode->addChild(geode);
            auto autoTransform = new osg::AutoTransform();
            // TODO allow pivot point parameterization
            autoTransform->setPivotPoint(osg::Vec3d(0.0, 0.0, 0.0));
//            autoTransform->setPivotPoint(osg::Vec3d(image->s() / 2, image->t() / 2, 0.0));
            autoTransform->setAutoScaleToScreen(true);
            autoTransform->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
            autoTransform->setPosition(osg::Vec3d(0.0, 0.0, boundingSphere.radius()));
            autoTransform->addChild(annotationNode);
            group->addChild(autoTransform);
        }
        osgNode = group;
    }
    else {
        const char *icon = displayString.getTagArg("i", 0);
        std::string path(icon);
        path += ".png";
        path = networkNode->resolveResourcePath(path.c_str());
        auto image = osgDB::readImageFile(path.c_str());
        if (image == nullptr)
            throw cRuntimeError("Cannot find icon '%s' at '%s'", icon, path.c_str());
        auto texture = new osg::Texture2D();
        texture->setImage(image);
        auto geometry = osg::createTexturedQuadGeometry(osg::Vec3(-image->s() / 2, 0.0, 0.0), osg::Vec3(image->s(), 0.0, 0.0), osg::Vec3(0.0, image->t(), 0.0), 0.0, 0.0, 1.0, 1.0);
        geometry->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture);
        geometry->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
        geometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
        geometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        auto geode = new osg::Geode();
        geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
        geode->addDrawable(geometry);
        if (displayModuleName) {
            label->setPosition(osg::Vec3(0.0, image->t() + spacing, 0.0));
            geode->addDrawable(label);
        }
        size = osg::Vec3d(image->s(), image->t() + spacing + 18, 0);
        annotationNode = new osg::Group();
        annotationNode->addChild(geode);
        auto autoTransform = new osg::AutoTransform();
        // TODO allow pivot point parameterization
        autoTransform->setPivotPoint(osg::Vec3d(0.0, 0.0, 0.0));
//        autoTransform->setPivotPoint(osg::Vec3d(image->s() / 2, image->t() / 2, 0.0));
        autoTransform->setAutoScaleToScreen(true);
        autoTransform->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
        autoTransform->setPosition(osg::Vec3d(0.0, 0.0, 1.0));
        autoTransform->addChild(annotationNode);
        osgNode = autoTransform;
    }
    auto objectNode = new cObjectOsgNode(networkNode);
    objectNode->addChild(osgNode);
    addChild(objectNode);
    double x = atol(displayString.getTagArg("p", 0));
    double y = atol(displayString.getTagArg("p", 1));
    setPosition(osg::Vec3d(x, y, 0.0));
}

std::string NetworkNodeOsgVisualization::getOsgModelPath(cModule *networkNode)
{
    const char *osgModel = networkNode->par("osgModel");
    auto firstDot = strchr(osgModel, '.');
    if (firstDot == nullptr)
        return std::string(osgModel);
    else {
        auto secondDot = strchr(firstDot + 1, '.');
        if (secondDot == nullptr)
            return networkNode->resolveResourcePath(osgModel);
        else {
            auto path = std::string(osgModel).substr(0, secondDot - osgModel);
            auto resolvedPath = networkNode->resolveResourcePath(path.c_str());
            auto pseudoLoader = std::string(osgModel).substr(secondDot - osgModel);
            return resolvedPath + pseudoLoader;
        }
    }
}

void NetworkNodeOsgVisualization::updateAnnotationPositions()
{
    double spacing = 4;
    double totalHeight = 0;
    for (auto annotation : annotations) {
        // TODO what should be the default pivot point? double dx = -annotation.size.x() / 2;
        double dx = 0;
        double dy = size.y() + spacing + totalHeight;
        auto positionAttitudeTransform = static_cast<osg::PositionAttitudeTransform *>(annotation.node->getParent(0));
        positionAttitudeTransform->setPosition(osg::Vec3d(dx, dy, 0));
        totalHeight += annotation.size.y() + spacing;
    }
}

void NetworkNodeOsgVisualization::addAnnotation(osg::Node *node, osg::Vec3d size, double priority)
{
    annotations.push_back(Annotation(node, size, priority));
    std::stable_sort(annotations.begin(), annotations.end(), [] (Annotation a1, Annotation a2) {
        return a1.priority < a2.priority;
    });
    auto positionAttitudeTransform = new osg::PositionAttitudeTransform();
    positionAttitudeTransform->addChild(node);
    annotationNode->addChild(positionAttitudeTransform);
    updateAnnotationPositions();
}

void NetworkNodeOsgVisualization::removeAnnotation(osg::Node *node)
{
    for (auto it = annotations.begin(); it != annotations.end(); it++) {
        if ((*it).node == node) {
            annotations.erase(it);
            break;
        }
    }
    annotationNode->removeChild(node->getParent(0));
    updateAnnotationPositions();
}

void NetworkNodeOsgVisualization::removeAnnotation(int index)
{
    auto it = annotations.begin() + index;
    auto node = (*it).node;
    annotations.erase(it);
    annotationNode->removeChild(node->getParent(0));
    updateAnnotationPositions();
}

} // namespace visualizer

} // namespace inet

