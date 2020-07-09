#pragma once

#include <vector>

struct Vertex
{
    Vertex() {}
    Vertex(float x, float y, float z) : x(x), y(y), z(z) {}

    float x;
    float y;
    float z;
};

struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};