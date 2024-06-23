#ifndef __fnVecMatDefines_h__
#define __fnVecMatDefines_h__

#include "vnl/vnl_matrix_fixed.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix.h"
#include "vnl/vnl_vector.h"

namespace Foundry
{
  namespace Math
  {

    template<class T, int N=0, int M=0>
    class MathTypes
    {
    public:
      
      // Template type
      typedef vnl_vector_fixed<T, N> Vec_t;
      typedef vnl_matrix_fixed<T, N, M> Mat_t;      
      
      typedef vnl_matrix<T> DynMat_t;
      typedef vnl_vector<T> DynVec_t;      
      
      //Fixed Size typedefs
      typedef vnl_vector_fixed<T, 2> Vec2;
      typedef vnl_vector_fixed<T, 3> Vec3;
      typedef vnl_vector_fixed<T, 4> Vec4;
      typedef vnl_matrix_fixed<T, 2, 2> Mat22;
      typedef vnl_matrix_fixed<T, 2, 3> Mat23;
      typedef vnl_matrix_fixed<T, 3, 2> Mat32;
      typedef vnl_matrix_fixed<T, 3, 3> Mat33;
      typedef vnl_matrix_fixed<T, 3, 4> Mat34;
      typedef vnl_matrix_fixed<T, 4, 4> Mat44;

      
      //Wrapping user memory typedefs
      typedef vnl_matrix<T> Matrix;
      typedef vnl_vector<T> Vector;

      //Wrapping complex type
      typedef vcl_complex<T> Complex;
    };

    typedef MathTypes<int>::Vec2 Vec2i;
    typedef MathTypes<unsigned int>::Vec2 Vec2ui;

    typedef MathTypes<float>::Vec2 Vec2f;
    typedef MathTypes<float>::Vec3 Vec3f;
    typedef MathTypes<float>::Vec4 Vec4f;

    typedef MathTypes<double>::Vec2 Vec2d;
    typedef MathTypes<double>::Vec3 Vec3d;
    typedef MathTypes<double>::Vec4 Vec4d;

    typedef MathTypes<float>::Mat22 Mat22f;
    typedef MathTypes<float>::Mat23 Mat23f;
    typedef MathTypes<float>::Mat32 Mat32f;
    typedef MathTypes<float>::Mat33 Mat33f;
    typedef MathTypes<float>::Mat34 Mat34f;
    typedef MathTypes<float>::Mat44 Mat44f;

    typedef MathTypes<double>::Mat22 Mat22d;
    typedef MathTypes<double>::Mat23 Mat23d;
    typedef MathTypes<double>::Mat32 Mat32d;
    typedef MathTypes<double>::Mat33 Mat33d;
    typedef MathTypes<double>::Mat34 Mat34d;
    typedef MathTypes<double>::Mat44 Mat44d;

    typedef MathTypes<float>::Matrix Matrixf;
    typedef MathTypes<double>::Matrix Matrixd;
    typedef MathTypes<float>::Vector Vectorf;
    typedef MathTypes<double>::Vector Vectord;

    typedef MathTypes<float>::Complex Complexf;
    typedef MathTypes<double>::Complex Complexd;

  }
}
#endif

