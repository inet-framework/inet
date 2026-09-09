//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/geometry/common/PlyPointCloudReader.h"

#include <cstring>
#include <fstream>
#include <sstream>

namespace inet {

static int plyTypeSize(const std::string& t)
{
    if (t == "double" || t == "float64") return 8;
    if (t == "float" || t == "float32" || t == "int" || t == "int32" || t == "uint" || t == "uint32") return 4;
    if (t == "short" || t == "int16" || t == "ushort" || t == "uint16") return 2;
    if (t == "char" || t == "int8" || t == "uchar" || t == "uint8") return 1;
    return 0;
}

static double plyRead(const char *p, const std::string& t)
{
    if (t == "double" || t == "float64") { double v; std::memcpy(&v, p, 8); return v; }
    if (t == "float" || t == "float32") { float v; std::memcpy(&v, p, 4); return (double)v; }
    if (t == "int" || t == "int32") { int32_t v; std::memcpy(&v, p, 4); return (double)v; }
    if (t == "uint" || t == "uint32") { uint32_t v; std::memcpy(&v, p, 4); return (double)v; }
    if (t == "short" || t == "int16") { int16_t v; std::memcpy(&v, p, 2); return (double)v; }
    if (t == "ushort" || t == "uint16") { uint16_t v; std::memcpy(&v, p, 2); return (double)v; }
    if (t == "char" || t == "int8") { int8_t v; std::memcpy(&v, p, 1); return (double)v; }
    if (t == "uchar" || t == "uint8") { uint8_t v; std::memcpy(&v, p, 1); return (double)v; }
    return 0.0;
}

PlyPointCloud PlyPointCloudReader::read(const std::string& path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in)
        throw cRuntimeError("Cannot open PLY file '%s' (resolved relative to the working directory)", path.c_str());

    // --- header ---
    std::string line;
    std::getline(in, line);
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (line.rfind("ply", 0) != 0)
        throw cRuntimeError("'%s' is not a PLY file (missing 'ply' magic)", path.c_str());
    std::string format;
    int vertexCount = 0;
    std::vector<std::pair<std::string, std::string>> props; // (type, name) of the vertex element, in order
    std::string curElement;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::istringstream ss(line);
        std::string tok; ss >> tok;
        if (tok == "format") ss >> format;
        else if (tok == "element") { ss >> curElement; if (curElement == "vertex") ss >> vertexCount; }
        else if (tok == "property" && curElement == "vertex") {
            std::string type; ss >> type;
            if (type == "list") continue; // e.g. face vertex-index lists — not a vertex scalar
            std::string name; ss >> name;
            props.emplace_back(type, name);
        }
        else if (tok == "end_header") break;
    }
    bool ascii = (format.rfind("ascii", 0) == 0);
    bool binaryLE = (format.rfind("binary_little_endian", 0) == 0);
    if (!ascii && !binaryLE)
        throw cRuntimeError("'%s' has unsupported PLY format '%s' (only ascii and binary_little_endian are supported)", path.c_str(), format.c_str());
    if (vertexCount <= 0 || props.empty())
        throw cRuntimeError("'%s' has no readable vertex element", path.c_str());

    // locate x/y/z (+ optional r/g/b) by property name; compute byte offsets and column indices
    int stride = 0, offX = -1, offY = -1, offZ = -1, offR = -1, offG = -1, offB = -1;
    int idxX = -1, idxY = -1, idxZ = -1, idxR = -1, idxG = -1, idxB = -1;
    std::string tX, tY, tZ, tR, tG, tB;
    for (size_t i = 0; i < props.size(); i++) {
        const std::string& ty = props[i].first;
        const std::string& n = props[i].second;
        if (n == "x") { offX = stride; tX = ty; idxX = (int)i; }
        else if (n == "y") { offY = stride; tY = ty; idxY = (int)i; }
        else if (n == "z") { offZ = stride; tZ = ty; idxZ = (int)i; }
        else if (n == "red" || n == "r") { offR = stride; tR = ty; idxR = (int)i; }
        else if (n == "green" || n == "g") { offG = stride; tG = ty; idxG = (int)i; }
        else if (n == "blue" || n == "b") { offB = stride; tB = ty; idxB = (int)i; }
        stride += plyTypeSize(ty);
    }
    if (offX < 0 || offY < 0 || offZ < 0)
        throw cRuntimeError("'%s' has no x/y/z vertex properties", path.c_str());
    PlyPointCloud cloud;
    cloud.hasRGB = (offR >= 0 && offG >= 0 && offB >= 0);
    // 8-bit channels are 0..255, float channels 0..1 — normalise each channel by ITS OWN type.
    auto colScale = [](const std::string& t) { return (t == "uchar" || t == "uint8") ? 1.0 / 255.0 : 1.0; };
    double scaleR = colScale(tR), scaleG = colScale(tG), scaleB = colScale(tB);

    // --- read the vertices ---
    cloud.xs.resize(vertexCount);
    cloud.ys.resize(vertexCount);
    cloud.zs.resize(vertexCount);
    if (cloud.hasRGB) {
        cloud.rs.resize(vertexCount);
        cloud.gs.resize(vertexCount);
        cloud.bs.resize(vertexCount);
    }
    if (binaryLE) {
        std::vector<char> buf(stride);
        for (int i = 0; i < vertexCount; i++) {
            in.read(buf.data(), stride);
            if (!in)
                throw cRuntimeError("'%s' is truncated (expected %d vertices)", path.c_str(), vertexCount);
            cloud.xs[i] = plyRead(buf.data() + offX, tX);
            cloud.ys[i] = plyRead(buf.data() + offY, tY);
            cloud.zs[i] = plyRead(buf.data() + offZ, tZ);
            if (cloud.hasRGB) {
                cloud.rs[i] = plyRead(buf.data() + offR, tR) * scaleR;
                cloud.gs[i] = plyRead(buf.data() + offG, tG) * scaleG;
                cloud.bs[i] = plyRead(buf.data() + offB, tB) * scaleB;
            }
        }
    }
    else { // ascii
        for (int i = 0; i < vertexCount; i++) {
            if (!std::getline(in, line))
                throw cRuntimeError("'%s' is truncated (expected %d vertices)", path.c_str(), vertexCount);
            std::istringstream ss(line);
            std::vector<double> v; double d; while (ss >> d) v.push_back(d);
            if ((int)v.size() < (int)props.size())
                throw cRuntimeError("'%s' has a short vertex line (%d values, expected %d)", path.c_str(), (int)v.size(), (int)props.size());
            cloud.xs[i] = v[idxX];
            cloud.ys[i] = v[idxY];
            cloud.zs[i] = v[idxZ];
            if (cloud.hasRGB) {
                cloud.rs[i] = v[idxR] * scaleR;
                cloud.gs[i] = v[idxG] * scaleG;
                cloud.bs[i] = v[idxB] * scaleB;
            }
        }
    }

    // --- bounding box ---
    cloud.minX = cloud.maxX = cloud.xs[0];
    cloud.minY = cloud.maxY = cloud.ys[0];
    cloud.minZ = cloud.maxZ = cloud.zs[0];
    for (int i = 1; i < vertexCount; i++) {
        cloud.minX = std::min(cloud.minX, cloud.xs[i]); cloud.maxX = std::max(cloud.maxX, cloud.xs[i]);
        cloud.minY = std::min(cloud.minY, cloud.ys[i]); cloud.maxY = std::max(cloud.maxY, cloud.ys[i]);
        cloud.minZ = std::min(cloud.minZ, cloud.zs[i]); cloud.maxZ = std::max(cloud.maxZ, cloud.zs[i]);
    }
    return cloud;
}

} // namespace inet
