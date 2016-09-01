#ifndef PRIMITIVES_HPP
#define PRIMITIVES_HPP

#include "domain.hpp"

#include <memory>
#include <vector>

namespace loom {

class Ball: public loom::DomainBase
{
  public:
    // argument are double (not K::FT) for Python compatibility
    Ball(
        const std::vector<double> & x0,
        const double radius
        ):
      x0_(x0),
      radius_(radius)
    {
      assert(x0_.size() == 3);
    }

    virtual
    double
    eval(const double x, const double y, const double z) const
    {
      const K::FT xx0 = x - x0_[0];
      const K::FT yy0 = y - x0_[1];
      const K::FT zz0 = z - x0_[2];
      return xx0*xx0 + yy0*yy0 + zz0*zz0 - radius_*radius_;
    }

    virtual
    double
    get_bounding_sphere_squared_radius() const
    {
      const double x0_nrm = sqrt(x0_[0]*x0_[0] + x0_[1]*x0_[1] + x0_[2]*x0_[2]);
      return (x0_nrm + radius_) * (x0_nrm + radius_);
    }

  private:
    const std::vector<double> x0_;
    const double radius_;
};


class Cuboid: public loom::DomainBase
{
  public:
    Cuboid(
        const std::vector<double> & x0,
        const std::vector<double> & x1
        ):
      x0_(x0),
      x1_(x1)
    {
    }

    virtual
    double
    eval(const double x, const double y, const double z) const
    {
      return (
          x0_[0] < x && x < x1_[0] &&
          x0_[1] < y && y < x1_[1] &&
          x0_[2] < z && z < x1_[2]
          ) ? -1.0 : 1.0;
    }

    virtual
    double
    get_bounding_sphere_squared_radius() const
    {
      const double x0_nrm2 = x0_[0]*x0_[0] + x0_[1]*x0_[1] + x0_[2]*x0_[2];
      const double x1_nrm2 = x1_[0]*x1_[0] + x1_[1]*x1_[1] + x1_[2]*x1_[2];
      return std::max({x0_nrm2, x1_nrm2});
    }

    virtual
    std::list<std::vector<K::Point_3>>
    get_features() const
    {
      std::vector<K::Point_3> corners = {
        K::Point_3(x0_[0], x0_[1], x0_[2]),
        K::Point_3(x1_[0], x0_[1], x0_[2]),
        K::Point_3(x0_[0], x1_[1], x0_[2]),
        K::Point_3(x0_[0], x0_[1], x1_[2]),
        K::Point_3(x1_[0], x1_[1], x0_[2]),
        K::Point_3(x1_[0], x0_[1], x1_[2]),
        K::Point_3(x0_[0], x1_[1], x1_[2]),
        K::Point_3(x1_[0], x1_[1], x1_[2])
      };
      return {
          {corners[0], corners[1]},
          {corners[0], corners[2]},
          {corners[0], corners[3]},
          {corners[1], corners[4]},
          {corners[1], corners[5]},
          {corners[2], corners[4]},
          {corners[2], corners[6]},
          {corners[3], corners[5]},
          {corners[3], corners[6]},
          {corners[4], corners[7]},
          {corners[5], corners[7]},
          {corners[6], corners[7]}
        };
    };

  private:
    const std::vector<double> x0_;
    const std::vector<double> x1_;
};


class Ellipsoid: public loom::DomainBase
{
  public:
    Ellipsoid(
        const std::vector<double> & x0,
        const double a0,
        const double a1,
        const double a2
        ):
      x0_(x0),
      a0_2_(a0*a0),
      a1_2_(a1*a1),
      a2_2_(a2*a1)
    {
    }

    virtual
    double
    eval(const double x, const double y, const double z) const
    {
      const K::FT xx0 = x - x0_[0];
      const K::FT yy0 = y - x0_[1];
      const K::FT zz0 = z - x0_[2];
      return xx0*xx0/a0_2_ + yy0*yy0/a1_2_ + zz0*zz0/a2_2_ - 1.0;
    }

    virtual
    double
    get_bounding_sphere_squared_radius() const
    {
      return std::max({a0_2_, a1_2_, a2_2_});
    }

  private:
    const std::vector<double> x0_;
    const double a0_2_;
    const double a1_2_;
    const double a2_2_;
};


class Cylinder: public loom::DomainBase
{
  public:
    Cylinder(
        const double z0,
        const double z1,
        const double radius,
        const double feature_edge_length
        ):
      z0_(z0),
      z1_(z1),
      radius_(radius),
      feature_edge_length_(feature_edge_length)
    {
      assert(z1_ > z0_);
    }

    virtual
    double
    eval(const double x, const double y, const double z) const
    {
      return (z0_ < z && z < z1_) ?
        x*x + y*y - radius_*radius_ :
        1.0;
    }

    virtual
    double
    get_bounding_sphere_squared_radius() const
    {
      const double zmax = std::max({abs(z0_), abs(z1_)});
      return zmax*zmax + radius_*radius_;
    }

    virtual
    std::list<std::vector<K::Point_3>>
    get_features() const
    {
      const double pi = 3.1415926535897932384;
      const size_t n = 2 * pi * radius_ / feature_edge_length_;
      std::vector<K::Point_3> circ0(n+1);
      std::vector<K::Point_3> circ1(n+1);
      for (size_t i=0; i < n; i++) {
        const double c = radius_ * cos((2*pi * i) / n);
        const double s = radius_ * sin((2*pi * i) / n);
        circ0[i] = K::Point_3(c, s, z0_);
        circ1[i] = K::Point_3(c, s, z1_);
      }
      // close the circles
      circ0[n] = circ0[0];
      circ1[n] = circ1[0];
      return {circ0, circ1};
    };

  private:
    const double z0_;
    const double z1_;
    const double radius_;
    const double feature_edge_length_;
};


class Cone: public loom::DomainBase
{
  public:
    Cone(
        const double radius,
        const double height,
        const double feature_edge_length
        ):
      radius_(radius),
      height_(height),
      feature_edge_length_(feature_edge_length)
    {
      assert(radius_ > 0.0);
      assert(height_ > 0.0);
    }

    virtual
    double
    eval(const double x, const double y, const double z) const
    {
      const K::FT rad = radius_ * (1.0 - z / height_);

      return (0.0 < z && z < height_) ?
        x*x + y*y - rad*rad :
        1.0;
    }

    virtual
    double
    get_bounding_sphere_squared_radius() const
    {
      const double max = std::max({radius_, height_});
      return max*max;
    }

    virtual
    std::list<std::vector<K::Point_3>>
    get_features() const
    {
      const double pi = 3.1415926535897932384;
      const size_t n = 2 * pi * radius_ / feature_edge_length_;
      std::vector<K::Point_3> circ0(n+1);
      for (size_t i=0; i < n; i++) {
        const double c = radius_ * cos((2*pi * i) / n);
        const double s = radius_ * sin((2*pi * i) / n);
        circ0[i] = K::Point_3(c, s, 0.0);
      }
      circ0[n] = circ0[0];
      return {circ0};
    };

  private:
    const double radius_;
    const double height_;
    const double feature_edge_length_;
};


class Tetrahedron: public loom::DomainBase
{
  public:
    Tetrahedron(
        const std::vector<double> & x0,
        const std::vector<double> & x1,
        const std::vector<double> & x2,
        const std::vector<double> & x3
        ):
      x0_(Eigen::Vector3d(x0[0], x0[1], x0[2])),
      x1_(Eigen::Vector3d(x1[0], x1[1], x1[2])),
      x2_(Eigen::Vector3d(x2[0], x2[1], x2[2])),
      x3_(Eigen::Vector3d(x3[0], x3[1], x3[2]))
    {
    }

    bool isOnSameSide(
        const Eigen::Vector3d & v0,
        const Eigen::Vector3d & v1,
        const Eigen::Vector3d & v2,
        const Eigen::Vector3d & v3,
        const Eigen::Vector3d & p
        ) const
    {
      const auto normal = (v1 - v0).cross(v2 - v0);
      const double dot_v3 = normal.dot(v3 - v0);
      const double dot_p = normal.dot(p - v0);
      return (
          (dot_v3 > 0 && dot_p > 0) || (dot_v3 < 0 && dot_p < 0)
          );
    }

    virtual
    double
    eval(const double x, const double y, const double z) const
    {
      Eigen::Vector3d pvec(x, y, z);
      const bool a =
        isOnSameSide(x0_, x1_, x2_, x3_, pvec) &&
        isOnSameSide(x1_, x2_, x3_, x0_, pvec) &&
        isOnSameSide(x2_, x3_, x0_, x1_, pvec) &&
        isOnSameSide(x3_, x0_, x1_, x2_, pvec);
      return a ? -1.0 : 1.0;
    }

    virtual
    double
    get_bounding_sphere_squared_radius() const
    {
      return std::max({
          x0_.dot(x0_),
          x1_.dot(x1_),
          x2_.dot(x2_),
          x3_.dot(x3_)
          });
    }

    virtual
    std::list<std::vector<K::Point_3>>
    get_features() const
    {
      std::vector<K::Point_3> pts = {
        K::Point_3(x0_[0], x0_[1], x0_[2]),
        K::Point_3(x1_[0], x1_[1], x1_[2]),
        K::Point_3(x2_[0], x2_[1], x2_[2]),
        K::Point_3(x3_[0], x3_[1], x3_[2]),
      };
      return {
          {pts[0], pts[1]},
          {pts[0], pts[2]},
          {pts[0], pts[3]},
          {pts[1], pts[2]},
          {pts[1], pts[3]},
          {pts[2], pts[3]}
        };
    };

  private:
    const Eigen::Vector3d x0_;
    const Eigen::Vector3d x1_;
    const Eigen::Vector3d x2_;
    const Eigen::Vector3d x3_;
};


class Torus: public loom::DomainBase
{
  public:
    Torus(
        const double major_radius,
        const double minor_radius
        ):
      major_radius_(major_radius),
      minor_radius_(minor_radius)
    {
    }

    virtual
    double
    eval(const double x, const double y, const double z) const
    {
      const double r = sqrt(x*x + y*y);
      return (r - major_radius_)*(r - major_radius_) + z*z <
        minor_radius_*minor_radius_ ?
        -1.0 :
        1.0;
    }

    virtual
    double
    get_bounding_sphere_squared_radius() const
    {
      return (major_radius_ + minor_radius_)*(major_radius_ + minor_radius_);
    }

  private:
    const double major_radius_;
    const double minor_radius_;
};

} // namespace loom

#endif // PRIMITIVES_HPP
