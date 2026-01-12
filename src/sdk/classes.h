#pragma once

struct Vector3 {
    float x, y, z;

    // --- constructors ---
    constexpr Vector3() : x(0), y(0), z(0) {}
    constexpr Vector3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}

    // --- operators ---
    constexpr Vector3 operator+(const Vector3& v) const {
        return { x + v.x, y + v.y, z + v.z };
    }

    constexpr Vector3 operator-(const Vector3& v) const {
        return { x - v.x, y - v.y, z - v.z };
    }

    constexpr Vector3 operator*(float s) const {
        return { x * s, y * s, z * s };
    }

    constexpr Vector3& operator+=(const Vector3& v) {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }

    constexpr Vector3& operator-=(const Vector3& v) {
        x -= v.x; y -= v.y; z -= v.z;
        return *this;
    }

    constexpr Vector3& operator*=(float s) {
        x *= s; y *= s; z *= s;
        return *this;
    }

    // --- helpers ---
    constexpr float dot(const Vector3& v) const {
        return x*v.x + y*v.y + z*v.z;
    }

    constexpr Vector3 cross(const Vector3& v) const {
        return {
            y*v.z - z*v.y,
            z*v.x - x*v.z,
            x*v.y - y*v.x
        };
    }

    // --- NEW: operator[] ---
    constexpr float operator[](int i) const {
        return (&x)[i];   // 0=x, 1=y, 2=z
    }

    constexpr float& operator[](int i) {
        return (&x)[i];
    }

    constexpr Vector3 Scale(float s) const {
        return (*this) * s;
    }

    float Dot(const Vector3& o) const
    {
        return x*o.x + y*o.y + z*o.z;
    }


};


class Vector3d
{
public:
    double x, y, z;
};

struct Matrix3x3 {
    Vector3 r[3]; // строки матрицы

    constexpr Matrix3x3() : r{ {1,0,0}, {0,1,0}, {0,0,1} } {}

    constexpr Matrix3x3(
        const Vector3& r0,
        const Vector3& r1,
        const Vector3& r2
    ) : r{ r0, r1, r2 } {}

    // доступ как m[row][col]
    constexpr float operator()(int row, int col) const {
        return (&r[row].x)[col];
    }

    const Vector3& operator[](int row) const { return r[row]; }
    Vector3& operator[](int row) { return r[row]; }

    // умножение матрицы на вектор
    constexpr Vector3 operator*(const Vector3& v) const {
        return {
            r[0].dot(v),
            r[1].dot(v),
            r[2].dot(v)
        };
    }
};

struct Matrix4x4 {
    float m[4][4];

    constexpr float* operator[](int i) {
        return m[i];
    }
    constexpr const float* operator[](int i) const {
        return m[i];
    }
};

struct ViewMatrix
{
	float* operator[](int index) noexcept
	{
		return data[index];
	}

	const float* operator[](int index) const noexcept
	{
		return data[index];
	}

	float data[4][4] = { };
};