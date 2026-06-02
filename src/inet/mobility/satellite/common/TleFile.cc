//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/mobility/satellite/common/TleFile.h"

#ifdef INET_WITH_SATELLITE_MOBILITY

#include <cstring>
#include <fstream>

namespace inet {

static std::string trim(const std::string& s)
{
    size_t b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos)
        return "";
    size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

static bool isElementLine(const std::string& line, char which)
{
    // a TLE element line starts with the digit '1' or '2' followed by a space
    return line.size() >= 2 && line[0] == which && line[1] == ' ';
}

void TleFile::load(const char *fileName, gravconsttype gravityConstants)
{
    std::ifstream in(fileName);
    if (!in)
        throw cRuntimeError("Cannot open TLE file '%s'", fileName);

    std::vector<std::string> lines;
    std::string raw;
    while (std::getline(in, raw))
        lines.push_back(raw);

    std::string pendingName;
    for (size_t i = 0; i < lines.size(); i++) {
        std::string line = lines[i];
        std::string trimmed = trim(line);
        if (trimmed.empty())
            continue;
        if (isElementLine(trimmed, '1') && i + 1 < lines.size() && isElementLine(trim(lines[i + 1]), '2')) {
            // twoline2rv() writes into the line buffers, so use mutable copies
            char longstr1[130], longstr2[130];
            strncpy(longstr1, lines[i].c_str(), sizeof(longstr1) - 1);
            longstr1[sizeof(longstr1) - 1] = '\0';
            strncpy(longstr2, lines[i + 1].c_str(), sizeof(longstr2) - 1);
            longstr2[sizeof(longstr2) - 1] = '\0';

            TleRecord record;
            double startmfe, stopmfe, deltamin;
            // typerun='c' (catalog) does not read from stdin; opsmode='i' is the improved mode
            SGP4Funcs::twoline2rv(longstr1, longstr2, 'c', 'c', 'i', gravityConstants,
                    startmfe, stopmfe, deltamin, record.satrec);
            if (record.satrec.error != 0)
                throw cRuntimeError("SGP4 initialization failed for satellite '%s' in '%s' (error code %d)",
                        pendingName.empty() ? record.satrec.satnum : pendingName.c_str(), fileName, record.satrec.error);
            record.name = !pendingName.empty() ? pendingName : std::string(record.satrec.satnum);
            records.push_back(record);
            pendingName.clear();
            i++; // also consumed line i+1
        }
        else if (!isElementLine(trimmed, '1') && !isElementLine(trimmed, '2'))
            // a non-element line preceding an element set is the satellite name
            pendingName = trimmed;
    }

    if (records.empty())
        throw cRuntimeError("No valid two-line element sets found in TLE file '%s'", fileName);
}

int TleFile::findByName(const char *name) const
{
    for (size_t i = 0; i < records.size(); i++)
        if (records[i].name == name)
            return (int)i;
    return -1;
}

int TleFile::findByCatalogNumber(const char *catalogNumber) const
{
    for (size_t i = 0; i < records.size(); i++)
        if (!strcmp(records[i].satrec.satnum, catalogNumber))
            return (int)i;
    return -1;
}

} // namespace inet

#endif // INET_WITH_SATELLITE_MOBILITY

