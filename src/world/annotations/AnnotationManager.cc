//
// AnnotationManager - manages annotations on the OMNeT++ canvas
// Copyright (C) 2010 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include <sstream>

#include "world/annotations/AnnotationManager.h"


Define_Module(AnnotationManager);

AnnotationManager::~AnnotationManager()
{

}

void AnnotationManager::initialize()
{
    debug = par("debug");

    annotations.clear();
    groups.clear();

    annotationsXml = par("annotations");
    addFromXml(annotationsXml);
}

void AnnotationManager::finish()
{
    hideAll();

    while (annotations.begin() != annotations.end())
    {
        delete *annotations.begin();
        annotations.erase(annotations.begin());
    }
    annotations.clear();

    while (groups.begin() != groups.end())
    {
        delete *groups.begin();
        groups.erase(groups.begin());
    }
    groups.clear();
}

void AnnotationManager::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        handleSelfMsg(msg);
        return;
    }
    error("AnnotationManager doesn't handle messages from other modules");
}

void AnnotationManager::handleSelfMsg(cMessage *msg)
{
    error("AnnotationManager doesn't handle self-messages");
}

void AnnotationManager::handleParameterChange(const char *parname)
{
    if (parname && (std::string(parname) == "draw"))
    {
        if (ev.isGUI() && par("draw"))
        {
            showAll();
        }
        else
        {
            hideAll();
        }
    }
}

/**
 * adds Annotations from an XML document; example below.
 *
 * <annotations>
 *   <line color="#F00" shape="16,0 8,13.8564" />
 *   <poly color="#0F0" shape="16,64 8,77.8564 -8,77.8564 -16,64 -8,50.1436 8,50.1436" />
 * </annotations>
 */
void AnnotationManager::addFromXml(cXMLElement* xml)
{
    std::string rootTag = xml->getTagName();
    ASSERT(rootTag == "annotations");

    cXMLElementList list = xml->getChildren();
    for (cXMLElementList::const_iterator i = list.begin(); i != list.end(); ++i)
    {
        cXMLElement* e = *i;

        std::string tag = e->getTagName();

        if (tag == "line")
        {
            ASSERT(e->getAttribute("color"));
            std::string color = e->getAttribute("color");
            ASSERT(e->getAttribute("shape"));
            std::string shape = e->getAttribute("shape");
            std::vector<std::string> points = cStringTokenizer(shape.c_str(), " ").asVector();
            ASSERT(points.size() == 2);

            std::vector<double> p1a = cStringTokenizer(points[0].c_str(), ",").asDoubleVector();
            ASSERT(p1a.size() == 2);
            std::vector<double> p2a = cStringTokenizer(points[1].c_str(), ",").asDoubleVector();
            ASSERT(p2a.size() == 2);

            drawLine(Coord(p1a[0], p1a[1]), Coord(p2a[0], p2a[1]), color);
        }
        else if (tag == "poly")
        {
            ASSERT(e->getAttribute("color"));
            std::string color = e->getAttribute("color");
            ASSERT(e->getAttribute("shape"));
            std::string shape = e->getAttribute("shape");
            std::vector<std::string> points = cStringTokenizer(shape.c_str(), " ").asVector();
            ASSERT(points.size() >= 2);

            std::vector<Coord> coords;
            for (std::vector<std::string>::const_iterator i = points.begin(); i != points.end(); ++i)
            {
                std::vector<double> pa = cStringTokenizer(i->c_str(), ",").asDoubleVector();
                ASSERT(pa.size() == 2);
                coords.push_back(Coord(pa[0], pa[1]));
            }

            drawPolygon(coords, color);
        }
        else
        {
            error("while reading annotations xml: expected 'line' or 'poly', but got '%s'", tag.c_str());
        }
    }

}

AnnotationManager::Group* AnnotationManager::createGroup(std::string title)
{
    Group* group = new Group(title);
    groups.push_back(group);

    return group;
}

AnnotationManager::Line* AnnotationManager::drawLine(Coord p1, Coord p2, std::string color, Group* group)
{
    Line* l = new Line(p1, p2, color);
    l->group = group;

    annotations.push_back(l);

    if (ev.isGUI() && par("draw")) show(l);

    return l;
}

AnnotationManager::Polygon* AnnotationManager::drawPolygon(std::list<Coord> coords, std::string color, Group* group)
{
    Polygon* p = new Polygon(coords, color);
    p->group = group;

    annotations.push_back(p);

    if (ev.isGUI() && par("draw")) show(p);

    return p;
}

AnnotationManager::Polygon* AnnotationManager::drawPolygon(std::vector<Coord> coords, std::string color, Group* group)
{
    return drawPolygon(std::list<Coord>(coords.begin(), coords.end()), color, group);
}

void AnnotationManager::drawBubble(Coord p1, std::string text)
{
    std::string pxOld = getDisplayString().getTagArg("p", 0);
    std::string pyOld = getDisplayString().getTagArg("p", 1);

    std::string px; { std::stringstream ss; ss << p1.x; px = ss.str(); }
    std::string py; { std::stringstream ss; ss << p1.y; py = ss.str(); }

    getDisplayString().setTagArg("p", 0, px.c_str());
    getDisplayString().setTagArg("p", 1, py.c_str());

    bubble(text.c_str());

    getDisplayString().setTagArg("p", 0, pxOld.c_str());
    getDisplayString().setTagArg("p", 1, pyOld.c_str());
}

void AnnotationManager::erase(const Annotation* annotation)
{
    hide(annotation);
    annotations.remove(const_cast<Annotation*>(annotation));
    delete annotation;
}

cModule* AnnotationManager::createDummyModule(std::string displayString)
{
    static int32_t nodeVectorIndex = -1;

    cModule* parentmod = getParentModule();
    if (!parentmod) error("Parent Module not found");

    cModuleType* nodeType = cModuleType::get("inet.world.annotations.AnnotationDummy");

    //TODO: this trashes the vectsize member of the cModule, although nobody seems to use it
    nodeVectorIndex++;
    cModule* mod = nodeType->create("annotation", parentmod, nodeVectorIndex, nodeVectorIndex);
    mod->finalizeParameters();
    mod->getDisplayString().parse(displayString.c_str());
    mod->buildInside();
    mod->scheduleStart(simTime());

    return mod;
}

cModule* AnnotationManager::createDummyModuleLine(Coord p1, Coord p2, std::string color)
{
    int w = abs(int(p2.x) - int(p1.x));
    int h = abs(int(p2.y) - int(p1.y));
    int px = 0;
    if (p1.x <= p2.x)
    {
        px = p1.x + 0.5 * w;
    }
    else
    {
        px = p2.x + 0.5 * w;
        w = -w;
    }
    int py = 0;
    if (p1.y <= p2.y)
    {
        py = p1.y + 0.5 * h;
    }
    else
    {
        py = p2.y + 0.5 * h;
        h = -h;
    }

    std::stringstream ss;
    ss << "p=" << px << "," << py << ";b=" << w << ", " << h << ",polygon," << color << "," << color << ",1";
    std::string displayString = ss.str();

    return createDummyModule(displayString);
}


void AnnotationManager::show(const Annotation* annotation)
{

    if (annotation->dummyObjects.size() > 0) return;

    if (const Line* l = dynamic_cast<const Line*>(annotation))
    {
        cModule* mod = createDummyModuleLine(l->p1, l->p2, l->color);

        annotation->dummyObjects.push_back(mod);
    }
    else if (const Polygon* p = dynamic_cast<const Polygon*>(annotation))
    {

        ASSERT(p->coords.size() >= 2);
        Coord lastCoords = *p->coords.rbegin();
        for (std::list<Coord>::const_iterator i = p->coords.begin(); i != p->coords.end(); ++i)
        {
            Coord c1 = *i;
            Coord c2 = lastCoords;
            lastCoords = c1;

            cModule* mod = createDummyModuleLine(c1, c2, p->color);
            annotation->dummyObjects.push_back(mod);
        }

    }
    else
    {
        error("unknown Annotation type");
    }

}

void AnnotationManager::hide(const Annotation* annotation)
{
    for (std::list<cModule*>::const_iterator i = annotation->dummyObjects.begin(); i != annotation->dummyObjects.end(); ++i)
    {
        cModule* mod = *i;
        mod->deleteModule();
    }
    annotation->dummyObjects.clear();
}

void AnnotationManager::showAll(Group* group)
{
    for (Annotations::const_iterator i = annotations.begin(); i != annotations.end(); ++i)
    {
        if ((!group) || ((*i)->group == group)) show(*i);
    }
}

void AnnotationManager::hideAll(Group* group)
{
    for (Annotations::const_iterator i = annotations.begin(); i != annotations.end(); ++i)
    {
        if ((!group) || ((*i)->group == group)) hide(*i);
    }
}
