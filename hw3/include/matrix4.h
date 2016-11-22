#ifndef MATRIX4_H
#define MATRIX4_H

#include <cassert>
#include <cmath>

#include "cvec.h"

// Forward declaration of Matrix4 and transpose since those are used below
class Matrix4;
Matrix4 transpose(const Matrix4& m);

// A 4x4 Matrix.
// To get the element at ith row and jth column, use a(i,j)
class Matrix4 {
  double d_[16]; // layout is row-major

public:
  double &operator () (const int row, const int col) {
    return d_[(row << 2) + col];
  }

  const double &operator () (const int row, const int col) const {
    return d_[(row << 2) + col];
  }

  double& operator [] (const int i) {
    return d_[i];
  }

  const double& operator [] (const int i) const {
    return d_[i];
  }

  Matrix4() {
    for (int i = 0; i < 16; ++i) {
      d_[i] = 0;
    }
    for (int i = 0; i < 4; ++i) {
      (*this)(i,i) = 1;
    }
  }

  Matrix4(const double a) {
    for (int i = 0; i < 16; ++i) {
      d_[i] = a;
    }
  }

  template <class T>
  Matrix4& readFromColumnMajorMatrix(const T m[]) {
    for (int i = 0; i < 16; ++i) {
      d_[i] = m[i];
    }
    return *this = transpose(*this);
  }

  template <class T>
  void writeToColumnMajorMatrix(T m[]) const {
    Matrix4 t = transpose(*this);
    for (int i = 0; i < 16; ++i) {
      m[i] = T(t.d_[i]);
    }
  }

  Matrix4& operator += (const Matrix4& m) {
    for (int i = 0; i < 16; ++i) {
      d_[i] += m.d_[i];
    }
    return *this;
  }

  Matrix4& operator -= (const Matrix4& m) {
    for (int i = 0; i < 16; ++i) {
      d_[i] -= m.d_[i];
    }
    return *this;
  }

  Matrix4& operator *= (const double a) {
    for (int i = 0; i < 16; ++i) {
      d_[i] *= a;
    }
    return *this;
  }

  Matrix4& operator *= (const Matrix4& a) {
    return *this = *this * a;
  }

  const Matrix4 operator + (const Matrix4& a) const {
    return Matrix4(*this) += a;
  }

  const Matrix4 operator - (const Matrix4& a) const {
    return Matrix4(*this) -= a;
  }

  const Matrix4 operator * (const double a) const {
    return Matrix4(*this) *= a;
  }

  const Cvec4 operator * (const Cvec4& v) const {
    Cvec4 r(0);
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        r[i] += (*this)(i,j) * v(j);
      }
    }
    return r;
  }

  const Matrix4 operator * (const Matrix4& m) const {
    Matrix4 r(0);
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        for (int k = 0; k < 4; ++k) {
          r(i,k) += (*this)(i,j) * m(j,k);
        }
      }
    }
    return r;
  }


  static const Matrix4 makeXRotation(const double angleInDegree) {
    return makeXRotation(std::cos(angleInDegree * CS175_PI/180), std::sin(angleInDegree * CS175_PI/180));
  }

  static const Matrix4 makeYRotation(const double angleInDegree) {
    return makeYRotation(std::cos(angleInDegree * CS175_PI/180), std::sin(angleInDegree * CS175_PI/180));
  }

  static const Matrix4 makeZRotation(const double angleInDegree) {
    return makeZRotation(std::cos(angleInDegree * CS175_PI/180), std::sin(angleInDegree * CS175_PI/180));
  }

  static const Matrix4 makeXRotation(const double c, const double s) {
    Matrix4 r;
    r(1,1) = r(2,2) = c;
    r(1,2) = -s;
    r(2,1) = s;
    return r;
  }

  static const Matrix4 makeYRotation(const double c, const double s) {
    Matrix4 r;
    r(0,0) = r(2,2) = c;
    r(0,2) = s;
    r(2,0) = -s;
    return r;
  }

  static const Matrix4 makeZRotation(const double c, const double s) {
    Matrix4 r;
    r(0,0) = r(1,1) = c;
    r(0,1) = -s;
    r(1,0) = s;
    return r;
  }

  static const Matrix4 makeTranslation(const Cvec3& t) {
    Matrix4 r;
    for (int i = 0; i < 3; ++i) {
      r(i,3) = t[i];
    }
    return r;
  }

  static const Matrix4 makeScale(const Cvec3& s) {
    Matrix4 r;
    for (int i = 0; i < 3; ++i) {
      r(i,i) = s[i];
    }
    return r;
  }

  static const Matrix4 makeProjection(
    const double top, const double bottom,
    const double left, const double right,
    const double nearClip, const double farClip) {
    Matrix4 r(0);
    // 1st row
    if (std::abs(right - left) > CS175_EPS) {
      r(0,0) = -2.0 * nearClip / (right - left);
      r(0,2) = (right+left) / (right - left);
    }
    // 2nd row
    if (std::abs(top - bottom) > CS175_EPS) {
      r(1,1) = -2.0 * nearClip / (top - bottom);
      r(1,2) = (top + bottom) / (top - bottom);
    }
    // 3rd row
    if (std::abs(farClip - nearClip) > CS175_EPS) {
      r(2,2) = (farClip+nearClip) / (farClip - nearClip);
      r(2,3) = -2.0 * farClip * nearClip / (farClip - nearClip);
    }
    r(3,2) = -1.0;
    return r;
  }

  static const Matrix4 makeProjection(const double fovy, const double aspectRatio, const double zNear, const double zFar) {
    Matrix4 r(0);
    const double ang = fovy * 0.5 * CS175_PI/180;
    const double f = std::abs(std::sin(ang)) < CS175_EPS ? 0 : 1/std::tan(ang);
    if (std::abs(aspectRatio) > CS175_EPS)
      r(0,0) = f/aspectRatio;  // 1st row

    r(1,1) = f;    // 2nd row

    if (std::abs(zFar - zNear) > CS175_EPS) { // 3rd row
      r(2,2) = (zFar+zNear) / (zFar - zNear);
      r(2,3) = -2.0 * zFar * zNear / (zFar - zNear);
    }

    r(3,2) = -1.0; // 4th row
    return r;
  }

};

inline bool isAffine(const Matrix4& m) {
  return std::abs(m[15]-1) + std::abs(m[14]) + std::abs(m[13]) + std::abs(m[12]) < CS175_EPS;
}

inline double norm2(const Matrix4& m) {
  double r = 0;
  for (int i = 0; i < 16; ++i) {
    r += m[i]*m[i];
  }
  return r;
}

// computes inverse of affine matrix. assumes last row is [0,0,0,1]
inline Matrix4 inv(const Matrix4& m) {
  Matrix4 r;                                              // default constructor initializes it to identity
  assert(isAffine(m));
  double det = m(0,0)*(m(1,1)*m(2,2) - m(1,2)*m(2,1)) +
               m(0,1)*(m(1,2)*m(2,0) - m(1,0)*m(2,2)) +
               m(0,2)*(m(1,0)*m(2,1) - m(1,1)*m(2,0));

  // check non-singular matrix
  assert(std::abs(det) > CS175_EPS3);

  // "rotation part"
  r(0,0) =  (m(1,1) * m(2,2) - m(1,2) * m(2,1)) / det;
  r(1,0) = -(m(1,0) * m(2,2) - m(1,2) * m(2,0)) / det;
  r(2,0) =  (m(1,0) * m(2,1) - m(1,1) * m(2,0)) / det;
  r(0,1) = -(m(0,1) * m(2,2) - m(0,2) * m(2,1)) / det;
  r(1,1) =  (m(0,0) * m(2,2) - m(0,2) * m(2,0)) / det;
  r(2,1) = -(m(0,0) * m(2,1) - m(0,1) * m(2,0)) / det;
  r(0,2) =  (m(0,1) * m(1,2) - m(0,2) * m(1,1)) / det;
  r(1,2) = -(m(0,0) * m(1,2) - m(0,2) * m(1,0)) / det;
  r(2,2) =  (m(0,0) * m(1,1) - m(0,1) * m(1,0)) / det;

  // "translation part" - multiply the translation (on the left) by the inverse linear part
  r(0,3) = -(m(0,3) * r(0,0) + m(1,3) * r(0,1) + m(2,3) * r(0,2));
  r(1,3) = -(m(0,3) * r(1,0) + m(1,3) * r(1,1) + m(2,3) * r(1,2));
  r(2,3) = -(m(0,3) * r(2,0) + m(1,3) * r(2,1) + m(2,3) * r(2,2));
  assert(isAffine(r) && norm2(Matrix4() - m*r) < CS175_EPS2);
  return r;
}

inline Matrix4 transpose(const Matrix4& m) {
  Matrix4 r(0);
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      r(i,j) = m(j,i);
    }
  }
  return r;
}

inline Matrix4 normalMatrix(const Matrix4& m) {
  Matrix4 invm = inv(m);
  invm(0, 3) = invm(1, 3) = invm(2, 3) = 0;
  return transpose(invm);
}

inline Matrix4 transFact(const Matrix4& m) {
	Matrix4 trans;
	trans[3]=m[3];
	trans[7]=m[7];
	trans[11]=m[11];
	return trans;
}

inline Matrix4 linFact(const Matrix4& m) {
	Matrix4 lin(m);
	lin[3]=lin[7]=lin[11]=0.0;
	return lin;
}

inline Matrix4 lookFrom(double eyex,double eyey,double eyez, double upx,double upy ,double upz ){
    Cvec3 z(eyex,eyey,eyez);
    Cvec3 up(upx,upy,upz);
    Cvec3 x = cross(up,z);
    Cvec3 y = cross(z,x);
    //normalize
    x=normalize(x);
    y=normalize(y);
    z=normalize(z);
    Matrix4 res;
    res(0,0)=x[0];res(0,1)=y[0];res(0,2)=z[0];res(0,3)=eyex;
    res(1,0)=x[1];res(1,1)=y[1];res(1,2)=z[1];res(1,3)=eyey;
    res(2,0)=x[2];res(2,1)=y[2];res(2,2)=z[2];res(2,3)=eyez;
    return res;
}
#endif

