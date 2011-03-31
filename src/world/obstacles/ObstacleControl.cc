//
// ObstacleControl - models obstacles that block radio transmissions
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
#include <map>
#include <set>

#include "world/obstacles/ObstacleControl.h"


Define_Module(ObstacleControl);

ObstacleControl::~ObstacleControl() {

}

void ObstacleControl::initialize(int stage) {
	if (stage == 1)	{
		debug = par("debug");

		obstacles.clear();
		cacheEntries.clear();

		annotations = AnnotationManagerAccess().getIfExists();
		if (annotations) annotationGroup = annotations->createGroup("obstacles");

		obstaclesXml = par("obstacles");

		addFromXml(obstaclesXml);
	}
}

void ObstacleControl::finish() {
	for (Obstacles::iterator i = obstacles.begin(); i != obstacles.end(); ++i) {
		for (ObstacleGridRow::iterator j = i->begin(); j != i->end(); ++j) {
			while (j->begin() != j->end()) erase(*j->begin());
		}
	}
	obstacles.clear();
}

void ObstacleControl::handleMessage(cMessage *msg) {
	if (msg->isSelfMessage()) {
		handleSelfMsg(msg);
		return;
	}
	error("ObstacleControl doesn't handle messages from other modules");
}

void ObstacleControl::handleSelfMsg(cMessage *msg) {
	error("ObstacleControl doesn't handle self-messages");
}

void ObstacleControl::addFromXml(cXMLElement* xml) {
	std::string rootTag = xml->getTagName();
	ASSERT (rootTag == "obstacles");

	cXMLElementList list = xml->getChildren();
	for (cXMLElementList::const_iterator i = list.begin(); i != list.end(); ++i) {
		cXMLElement* e = *i;

		std::string tag = e->getTagName();
		ASSERT(tag == "poly");

		// <poly id="building#0" type="building" color="#F00" shape="16,0 8,13.8564 -8,13.8564 -16,0 -8,-13.8564 8,-13.8564" />
		ASSERT(e->getAttribute("id"));
		std::string id = e->getAttribute("id");
		ASSERT(e->getAttribute("type"));
		std::string type = e->getAttribute("type");
		ASSERT(e->getAttribute("color"));
		std::string color = e->getAttribute("color");
		ASSERT(e->getAttribute("shape"));
		std::string shape = e->getAttribute("shape");

		double attenuationPerWall = 50; /**< in dB */
		double attenuationPerMeter = 1; /**< in dB / m */
		if (type == "building") { attenuationPerWall = 50; attenuationPerMeter = 1; }
		else error("unknown obstacle type: %s", type.c_str());
		Obstacle obs(id, attenuationPerWall, attenuationPerMeter);
		std::vector<Coord> sh;
		cStringTokenizer st(shape.c_str());
		while (st.hasMoreTokens()) {
			std::string xy = st.nextToken();
			std::vector<double> xya = cStringTokenizer(xy.c_str(), ",").asDoubleVector();
			ASSERT(xya.size() == 2);
			sh.push_back(Coord(xya[0], xya[1]));
		}
		obs.setShape(sh);
		add(obs);

	}

}

void ObstacleControl::add(Obstacle obstacle) {
	Obstacle* o = new Obstacle(obstacle);

	size_t fromRow = std::max(0, int(o->getBboxP1().x / GRIDCELL_SIZE));
	size_t toRow = std::max(0, int(o->getBboxP2().x / GRIDCELL_SIZE));
	size_t fromCol = std::max(0, int(o->getBboxP1().y / GRIDCELL_SIZE));
	size_t toCol = std::max(0, int(o->getBboxP2().y / GRIDCELL_SIZE));
	for (size_t row = fromRow; row <= toRow; ++row) {
		for (size_t col = fromCol; col <= toCol; ++col) {
			if (obstacles.size() < col+1) obstacles.resize(col+1);
			if (obstacles[col].size() < row+1) obstacles[col].resize(row+1);
			(obstacles[col])[row].push_back(o);
		}
	}

	// visualize using AnnotationManager
	if (annotations) o->visualRepresentation = annotations->drawPolygon(o->getShape(), "red", annotationGroup);

	cacheEntries.clear();
}

void ObstacleControl::erase(const Obstacle* obstacle) {
	for (Obstacles::iterator i = obstacles.begin(); i != obstacles.end(); ++i) {
		for (ObstacleGridRow::iterator j = i->begin(); j != i->end(); ++j) {
			for (ObstacleGridCell::iterator k = j->begin(); k != j->end(); ) {
				Obstacle* o = *k;
				if (o == obstacle) {
					k = j->erase(k);
				} else {
					++k;
				}
			}
		}
	}

	if (annotations && obstacle->visualRepresentation) annotations->erase(obstacle->visualRepresentation);
	delete obstacle;

	cacheEntries.clear();
}

double ObstacleControl::calculateReceivedPower(double pSend, double carrierFrequency, const Coord& senderPos, double senderAngle, const Coord& receiverPos, double receiverAngle) const {
	Enter_Method_Silent();

	// return cached result, if available
	CacheKey cacheKey(pSend, carrierFrequency, senderPos, senderAngle, receiverPos, receiverAngle);
	CacheEntries::const_iterator cacheEntryIter = cacheEntries.find(cacheKey);
	if (cacheEntryIter != cacheEntries.end()) return cacheEntryIter->second;

	// calculate bounding box of transmission
	Coord bboxP1 = Coord(std::min(senderPos.x, receiverPos.x), std::min(senderPos.y, receiverPos.y));
	Coord bboxP2 = Coord(std::max(senderPos.x, receiverPos.x), std::max(senderPos.y, receiverPos.y));

	size_t fromRow = std::max(0, int(bboxP1.x / GRIDCELL_SIZE));
	size_t toRow = std::max(0, int(bboxP2.x / GRIDCELL_SIZE));
	size_t fromCol = std::max(0, int(bboxP1.y / GRIDCELL_SIZE));
	size_t toCol = std::max(0, int(bboxP2.y / GRIDCELL_SIZE));

	std::set<Obstacle*> processedObstacles;
	for (size_t col = fromCol; col <= toCol; ++col) {
		if (col >= obstacles.size()) break;
		for (size_t row = fromRow; row <= toRow; ++row) {
			if (row >= obstacles[col].size()) break;
			const ObstacleGridCell& cell = (obstacles[col])[row];
			for (ObstacleGridCell::const_iterator k = cell.begin(); k != cell.end(); ++k) {

				Obstacle* o = *k;

				if (processedObstacles.find(o) != processedObstacles.end()) continue;
				processedObstacles.insert(o);

				// bail if bounding boxes cannot overlap
				if (o->getBboxP2().x < bboxP1.x) continue;
				if (o->getBboxP1().x > bboxP2.x) continue;
				if (o->getBboxP2().y < bboxP1.y) continue;
				if (o->getBboxP1().y > bboxP2.y) continue;

				double pSendOld = pSend;

				pSend = o->calculateReceivedPower(pSend, carrierFrequency, senderPos, senderAngle, receiverPos, receiverAngle);

				// draw a "hit!" bubble
				if (annotations && (pSend < pSendOld)) annotations->drawBubble(o->getBboxP1(), "hit");

				// bail if attenuation is already extremely high
				if (pSend < 1e-30) break;

			}
		}
	}

	// cache result
	if (cacheEntries.size() >= 1000) cacheEntries.clear();
	cacheEntries[cacheKey] = pSend;

	return pSend;
}
