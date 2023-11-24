#include "cube.h"

// Constructor implementation
Cube::Cube(const glm::vec3& min, const glm::vec3& max, const Material& mat)
    : min(min), max(max), Object(mat) {}

// Optimized Ray intersection method
Intersect Cube::rayIntersect(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) const {
    glm::vec3 invDir = 1.0f / rayDirection;
    glm::vec3 t0 = (min - rayOrigin) * invDir;
    glm::vec3 t1 = (max - rayOrigin) * invDir;

    glm::vec3 tmin = glm::min(t0, t1);
    glm::vec3 tmax = glm::max(t0, t1);

    float tNear = glm::max(tmin.x, glm::max(tmin.y, tmin.z));
    float tFar = glm::min(tmax.x, glm::min(tmax.y, tmax.z));

    if (tNear > tFar || tFar < 0) {
        return Intersect{false}; // No intersection
    }

    glm::vec3 intersectPoint = rayOrigin + tNear * rayDirection;
    return Intersect{intersectPoint, calculateNormal(intersectPoint), tNear};
}

// Optimized calculateNormal method
glm::vec3 Cube::calculateNormal(const glm::vec3& intersectPoint) const {
    const float bias = 1e-4; // Small bias to handle numerical precision issues
    glm::vec3 normal = glm::vec3(0.0f);

    if (std::fabs(intersectPoint.x - min.x) < bias) normal.x = -1.0f;
    else if (std::fabs(intersectPoint.x - max.x) < bias) normal.x = 1.0f;
    else if (std::fabs(intersectPoint.y - min.y) < bias) normal.y = -1.0f;
    else if (std::fabs(intersectPoint.y - max.y) < bias) normal.y = 1.0f;
    else if (std::fabs(intersectPoint.z - min.z) < bias) normal.z = -1.0f;
    else if (std::fabs(intersectPoint.z - max.z) < bias) normal.z = 1.0f;

    return normal;
}