#ifndef __TRANSFORM_H
#define __TRANSFORM_H

#include <bvh/bvh.hpp>
#include <bvh/triangle.hpp>


template <typename Scalar>
class Transform {
  public:
    Scalar a[3][3];
    bvh::Vector3<Scalar> v;
    Transform();
    Transform(Transform<Scalar> &t);
    Transform<Scalar> rotate(bvh::Vector3<Scalar> axis, Scalar angle) const;
    Transform<Scalar> scale(Scalar scale) const;
    Transform<Scalar> translate(bvh::Vector3<Scalar> translate) const;
    bvh::Vector3<Scalar> operator()(const bvh::Vector3<Scalar>& p) const;
};

template <typename Scalar>
Transform<Scalar>::Transform() {
    a[0][0] = 1;
    a[0][1] = 0;
    a[0][2] = 0;
    a[1][0] = 0;
    a[1][1] = 1;
    a[1][2] = 0;
    a[2][0] = 0;
    a[2][1] = 0;
    a[2][2] = 1;

    v = bvh::Vector3<Scalar>(0,0,0);
}

template <typename Scalar>
Transform<Scalar>::Transform(Transform<Scalar> &t) {
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            a[row][col] = t.a[row][col];
        }
    }

    v = t.v;
}

template <typename Scalar>
Transform<Scalar> Transform<Scalar>::scale(Scalar scale) const {
    Transform<Scalar> ret(this);
    
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            ret.a[row][col] *= scale;
        }
    }

    return ret;
}

template <typename Scalar>
Transform<Scalar> Transform<Scalar>::translate(bvh::Vector3<Scalar> translate) const {
    Transform<Scalar> ret(this);
    ret.v = ret.v + translate;
    return ret;
}

template <typename Scalar>
Transform<Scalar> Transform<Scalar>::rotate(bvh::Vector3<Scalar> axis, Scalar angle) const {
    Transform<Scalar> ret;

    ret.a[0][0] = 0;
    ret.a[1][1] = 0;
    ret.a[2][2] = 0;

    auto n_axis = bvh::normalize(axis);

    // Markley and Crassidis (Fund. of Spacecraft AD&C), pg 42
    Scalar s = std::sin(angle);
    Scalar c = std::cos(angle);
    Scalar mat[3][3] = {
        {
            c+(1-c)*n_axis[0]*n_axis[0],
            (1-c)*n_axis[0]*n_axis[1]+s*n_axis[2],
            (1-c)*n_axis[0]*n_axis[2]-s*n_axis[1],
        },
        {
            (1-c)*n_axis[1]*n_axis[0]-s*n_axis[2],
            c+(1-c)*n_axis[1]*n_axis[1],
            (1-c)*n_axis[1]*n_axis[2]+s*n_axis[0],
        },
        {
            (1-c)*n_axis[2]*n_axis[0]+s*n_axis[1],
            (1-c)*n_axis[2]*n_axis[1]-s*n_axis[0],
            c+(1-c)*n_axis[2]*n_axis[2],
        }
    };
    
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            for (int i = 0; i < 3; i++) {
                ret.a[row][col] += a[row][i] * mat[i][col];
            }
        }
    }

    ret.v = v;

    return ret;
}

template <typename Scalar>
bvh::Vector3<Scalar> Transform<Scalar>::operator()(const bvh::Vector3<Scalar>& p) const {
    return bvh::Vector3<Scalar>(
        a[0][0]*p[0] + a[0][1]*p[1] + a[0][2]*p[2] + v[0],
        a[1][0]*p[0] + a[1][1]*p[1] + a[1][2]*p[2] + v[1],
        a[2][0]*p[0] + a[2][1]*p[1] + a[2][2]*p[2] + v[2]
    );
}

template <typename Scalar>
static void transform_triangles(const Transform<Scalar> &transform, bvh::Triangle<Scalar>* triangles, size_t triangle_count) {
    // #pragma omp parallel for
    for (size_t i = 0; i < triangle_count; ++i) {
        auto p0 = transform(triangles[i].p0);
        auto p1 = transform(triangles[i].p1());
        auto p2 = transform(triangles[i].p2());
        triangles[i] = bvh::Triangle<Scalar>(p0, p1, p2);
    }
}

#endif