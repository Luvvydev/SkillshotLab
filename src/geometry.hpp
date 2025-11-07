#pragma once

#include <cmath>
#include <ostream>

namespace SkillshotLab {

struct Vec2 {
    float x{0.0f};
    float y{0.0f};

    Vec2() = default;
    Vec2(float x_, float y_) : x(x_), y(y_) {}

    Vec2 operator+(const Vec2& other) const {
        return Vec2{x + other.x, y + other.y};
    }

    Vec2 operator-(const Vec2& other) const {
        return Vec2{x - other.x, y - other.y};
    }

    Vec2 operator*(float s) const {
        return Vec2{x * s, y * s};
    }

    Vec2& operator+=(const Vec2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }
};

inline float length(const Vec2& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

inline Vec2 normalize(const Vec2& v) {
    float len = length(v);
    if (len <= 1e-4f) {
        return Vec2{0.0f, 0.0f};
    }
    return Vec2{v.x / len, v.y / len};
}

inline float distance(const Vec2& a, const Vec2& b) {
    return length(a - b);
}

inline float dot(const Vec2& a, const Vec2& b) {
    return a.x * b.x + a.y * b.y;
}

inline float cross(const Vec2& a, const Vec2& b) {
    return a.x * b.y - a.y * b.x;
}

inline std::ostream& operator<<(std::ostream& os, const Vec2& v) {
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}

} // namespace SkillshotLab
