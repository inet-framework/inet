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


#ifndef WORLD_ANNOTATION_ANNOTATIONCONTROL_H
#define WORLD_ANNOTATION_ANNOTATIONCONTROL_H

#include <list>
#include <vector>

#include "INETDefs.h"
#include "ModuleAccess.h"
#include "Coord.h"

/**
 * manages annotations on the OMNeT++ canvas.
 */
class INET_API AnnotationManager : public cSimpleModule
{
  public:
    class Group;
    typedef std::list<Coord> CoordList;
    typedef std::vector<Coord> CoordVector;

    class Annotation
    {
      public:
        Annotation() : group(0) {}
        virtual ~Annotation() {}

      protected:
        friend class AnnotationManager;

        Group* group;
        mutable std::list<cModule*> dummyObjects;
    };

    class Line : public Annotation
    {
      public:
        Line(Coord p1, Coord p2, std::string color) : p1(p1), p2(p2), color(color) {}
        virtual ~Line() {}

      protected:
        friend class AnnotationManager;

        Coord p1;
        Coord p2;
        std::string color;
    };

    class Polygon : public Annotation
    {
      public:
        Polygon(CoordList coords, std::string color) : coords(coords), color(color) {}
        virtual ~Polygon() {}

      protected:
        friend class AnnotationManager;

        CoordList coords;
        std::string color;
    };

    class Group
    {
      public:
        Group(std::string title) : title(title) {}
        virtual ~Group() {}

      protected:
        friend class AnnotationManager;

        std::string title;
    };

    ~AnnotationManager();

    /**
     * adds Annotations from an XML document; example below.
     *
     * <annotations>
     *   <line color="#F00" shape="16,0 8,13.8564" />
     *   <poly color="#0F0" shape="16,64 8,77.8564 -8,77.8564 -16,64 -8,50.1436 8,50.1436" />
     * </annotations>
     */
    void addFromXml(cXMLElement* xml);
    Group* createGroup(std::string title = "untitled");
    Line* drawLine(Coord p1, Coord p2, std::string color, Group* group = 0);
    Polygon* drawPolygon(CoordList coords, std::string color, Group* group = 0);
    Polygon* drawPolygon(CoordVector coords, std::string color, Group* group = 0);
    void drawBubble(Coord p1, std::string text);
    void erase(const Annotation* annotation);

    void show(const Annotation* annotation);
    void hide(const Annotation* annotation);
    void showAll(Group* group = 0);
    void hideAll(Group* group = 0);

  protected:
    void initialize();
    void finish();
    void handleMessage(cMessage *msg);
    void handleSelfMsg(cMessage *msg);
    void handleParameterChange(const char *parname);
    cModule* createDummyModule(std::string displayString);
    cModule* createDummyModuleLine(Coord p1, Coord p2, std::string color);

  protected:
    typedef std::list<Annotation*> Annotations;
    typedef std::list<Group*> Groups;

    cXMLElement* annotationsXml; /**< annotations to add at startup */
    Annotations annotations;
    Groups groups;
};

class AnnotationManagerAccess
{
  public:
    AnnotationManagerAccess() {}

    AnnotationManager* getIfExists()
    {
        return dynamic_cast<AnnotationManager*>(simulation.getModuleByPath("annotations"));
    }
};

#endif

