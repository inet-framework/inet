//
// Copyright (C) 2006 Andras Varga, Levente Meszaros
// Based on the Mobility Framework's SnrEval by Marc Loebbers
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

#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

#include "inet/common/INETDefs.h"
#include "inet/physicallayer/ieee80211/BerParseFile.h"

namespace inet {

namespace physicallayer {

void BerParseFile::clearBerTable()
{
    while (!berTable.empty()) {
        BerList *berList = &berTable.back();
        while (!berList->empty()) {
            delete berList->back();
            berList->pop_back();
        }
        berTable.pop_back();
    }
    fileBer = false;
}

void BerParseFile::setPhyOpMode(char p)
{
    clearBerTable();
    phyOpMode = p;
    if (phyOpMode == 'b')
        berTable.resize(4);
    else
        berTable.resize(8);
}

int BerParseFile::getTablePosition(double speed)
{
    speed /= 1000000;
    if (phyOpMode == 'b') {
        if (speed < 2)
            return 0;
        else if (speed < 5)
            return 1;
        else if (speed < 11)
            return 2;
        else
            return 3;
    }
    else {
        if (speed < 9)
            return 0;
        else if (speed < 12)
            return 1;
        else if (speed < 18)
            return 2;
        else if (speed < 24)
            return 3;
        else if (speed < 36)
            return 4;
        else if (speed < 48)
            return 5;
        else if (speed < 54)
            return 6;
        else
            return 7;
    }
}

double BerParseFile::getPer(double speed, double tsnr, int tlen)
{
    BerList *berlist;
    berlist = &berTable[getTablePosition(speed)];
    LongBer *pre = NULL;
    LongBer *pos = NULL;
    unsigned int j;
    for (j = 0; j < berlist->size(); j++) {
        pos = *(berlist->begin() + j);
        if (pos->longpkt >= tlen) {
            break;
        }
    }
    if (j == 0)
        pre = NULL;
    else {
        if (j == berlist->size())
            pre = *(berlist->begin() + j - 2);
        else
            pre = *(berlist->begin() + j - 1);
    }
    SnrBer snrdata1;
    SnrBer snrdata2;
    SnrBer snrdata3;
    SnrBer snrdata4;
    snrdata1.snr = -1;
    snrdata1.ber = -1;
    snrdata2.snr = -1;
    snrdata2.ber = -1;
    snrdata3.snr = -1;
    snrdata3.ber = -1;
    snrdata4.snr = -1;
    snrdata4.ber = -1;

    if (pos->snrlist.size() < 1)
        throw cRuntimeError("model error: pos->snrlist is empty");
    if (tsnr > pos->snrlist[pos->snrlist.size() - 1].snr) {
        snrdata1 = pos->snrlist[pos->snrlist.size() - 1];
        snrdata2 = pos->snrlist[pos->snrlist.size() - 1];
    }
    else {
        for (j = 0; j < pos->snrlist.size(); j++) {
            snrdata1 = pos->snrlist[j];
            if (tsnr <= snrdata1.snr)
                break;
        }
        if (j == 0) {
            snrdata2.snr = -1;
            snrdata2.ber = -1;
        }
        else {
            if (j == pos->snrlist.size()) {
                if (j < 2)
                    throw cRuntimeError("model error: pos->snrlist is too short, should be 2 or more elements");
                snrdata2 = pos->snrlist[j - 2];
            }
            else
                snrdata2 = pos->snrlist[j - 1];
        }
    }

    if (pre == NULL)
        pre = pos;
    if (tsnr > pre->snrlist[pre->snrlist.size() - 1].snr) {
        snrdata3 = pre->snrlist[pre->snrlist.size() - 1];
        snrdata4 = pre->snrlist[pre->snrlist.size() - 1];
    }
    else {
        for (j = 0; j < pre->snrlist.size(); j++) {
            snrdata3 = pre->snrlist[j];
            if (tsnr <= snrdata3.snr)
                break;
        }
        if (j != 0) {
            if (j == pre->snrlist.size())
                snrdata4 = pre->snrlist[j - 2];
            else
                snrdata4 = pre->snrlist[j - 1];
        }
    }
    if (snrdata2.snr == -1) {
        snrdata2.snr = snrdata1.snr;
        snrdata2.ber = snrdata1.ber;
    }
    if (snrdata4.snr == -1) {
        snrdata4.snr = snrdata3.snr;
        snrdata4.ber = snrdata3.ber;
    }
    double per1, per2, per;
    per1 = snrdata1.ber;
    per2 = snrdata3.ber;

    if (tsnr <= snrdata1.snr) {
        if (snrdata2.snr != snrdata1.snr)
            per1 = snrdata1.ber + (snrdata2.ber - snrdata1.ber) / (snrdata2.snr - snrdata1.snr) * (tsnr - snrdata1.snr);
    }
    if (tsnr <= snrdata3.snr) {
        if (snrdata3.snr != snrdata4.snr)
            per2 = snrdata3.ber + (snrdata4.ber - snrdata3.ber) / (snrdata4.snr - snrdata3.snr) * (tsnr - snrdata3.snr);
    }
    if (per1 != -1 && per2 != -1) {
        if (pos->longpkt != pre->longpkt)
            per = per2 + (per1 - per2) / (pos->longpkt - pre->longpkt) * (tlen - pre->longpkt);
        else
            per = per2;
    }
    else {
        if (per1 != -1) {
            per = per1;
        }
        else {
            if (per2 != -1) {
                per = per2;
            }
            else {
                EV << "No PER available";
                per = 0;
            }
        }
    }
    return per;
}

void BerParseFile::parseFile(const char *filename)
{
    std::ifstream in(filename, std::ios::in);
    if (in.fail())
        throw cRuntimeError("Cannot open file '%s'", filename);
    std::string line;
    std::string subline;

    while (std::getline(in, line)) {
        // '#' line
        std::string::size_type found = line.find('#');
        if (found == 0)
            continue;
        if (found != std::string::npos)
            subline = line;
        else
            subline = line.substr(0, found);
        found = subline.find("$self");
        if (found == std::string::npos)
            continue;
        // Node Id
        found = subline.find("add");
        if (found == std::string::npos)
            continue;
        // Initial position

        std::string::size_type pos1 = subline.find("Mode");
        std::string::size_type pos2 = subline.find("Mb");
        BerList *berlist;
        std::string substr = subline.substr(pos1 + 4, pos2 - (pos1 + 4));
//                int speed = std::atof (subline.substr(pos1+4,pos2).c_str());

        int position = -1;
        if (phyOpMode == 'b') {
            if (!strcmp(substr.c_str(), "1"))
                position = 0;
            else if (!strcmp(substr.c_str(), "2"))
                position = 1;
            else if (!strcmp(substr.c_str(), "5_5"))
                position = 2;
            else if (!strcmp(substr.c_str(), "11"))
                position = 3;
            else
                continue;
        }
        else {
            if (!strcmp(substr.c_str(), "6"))
                position = 0;
            else if (!strcmp(substr.c_str(), "9"))
                position = 1;
            else if (!strcmp(substr.c_str(), "12"))
                position = 2;
            else if (!strcmp(substr.c_str(), "18"))
                position = 3;
            else if (!strcmp(substr.c_str(), "24"))
                position = 4;
            else if (!strcmp(substr.c_str(), "36"))
                position = 5;
            else if (!strcmp(substr.c_str(), "48"))
                position = 6;
            else if (!strcmp(substr.c_str(), "54"))
                position = 7;
            else
                continue;
        }

        if (position < 0)
            throw cRuntimeError("ber parse file error");

        berlist = &berTable[position];

        std::string parameters = subline.substr(pos2 + 3, std::string::npos);
        std::stringstream linestream(parameters);
        int pkSize;
        double snr;
        double ber;
        linestream >> pkSize;
        linestream >> snr;
        linestream >> ber;
        snr = dB2fraction(snr);
        if (pkSize < 128)
            pkSize = 128;
        else if (128 <= pkSize && pkSize < 256)
            pkSize = 128;
        else if (256 <= pkSize && pkSize < 512)
            pkSize = 256;
        else if (512 <= pkSize && pkSize < 1024)
            pkSize = 512;
        else if (1024 <= pkSize && pkSize < 1500)
            pkSize = 1024;
        else
            pkSize = 1500;
        if (berlist->size() == 0) {
            LongBer *l = new LongBer;
            l->longpkt = pkSize;
            berlist->push_back(l);
        }
        LongBer *l;
        bool inList = false;
        for (unsigned int j = 0; j < berlist->size(); j++) {
            l = *(berlist->begin() + j);
            if (l->longpkt == pkSize) {
                inList = true;
                break;
            }
        }
        if (!inList) {
            l = new LongBer;
            l->longpkt = pkSize;

            unsigned int position = 0;
            for (position = 0; position < berlist->size(); position++) {
                LongBer *aux = *(berlist->begin() + position);

                if (l->longpkt < aux->longpkt)
                    break;
            }
            if (position == berlist->size())
                berlist->push_back(l);
            else
                berlist->insert(berlist->begin() + position, l);
        }
        SnrBer snrdata;
        snrdata.snr = snr;
        snrdata.ber = ber;
        l->snrlist.push_back(snrdata);
        std::sort(l->snrlist.begin(), l->snrlist.end(), std::less<SnrBer>());
    }
    in.close();

    // exist data?
    if (phyOpMode == 'b') {
        for (int i = 0; i < 4; i++)
            if (berTable[i].size() == 0)
                throw cRuntimeError("Error in ber class B file, speed not present");

    }
    else {
        for (int i = 0; i < 8; i++)
            if (berTable[i].size() == 0)
                throw cRuntimeError("Error in ber class A/G file, speed not present");

    }
}

BerParseFile::~BerParseFile()
{
    clearBerTable();
}

} // namespace physicallayer

} // namespace inet

