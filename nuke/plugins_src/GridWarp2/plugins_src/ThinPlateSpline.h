/*! \class DD::Math::ThinPlateSpline ThinPlateSpline.H

  \brief This class implements a method for interpolating between arbitrarily
  spaced values on a 2D surface. The technique simulates how a thin plate of
  metal would bend while still intersecting the points.

  It's called a "Thin Plate Spline" and it minimizes the bending energy of the
  surface going through the points. If you give this class a set of points
  where x and y are the location of the point on a 2D grid and z is the height
  of the point (note the coordinate system), this class will compute the
  appropriate paramters for the minimization and let you query the surface at
  any point.

  There is a problem though. If all the points are co-linear, the surface
  degenerates. Be careful. The class does a special case check if there is
  only one point, but will fail if there are only two points.

  Notice, this is a template class. I like Inventor, other people like
  Vector4. So, This class is templated so you can create it so it uses either
  SbVec3f or Vector4. (Or any class that supports x,y,z access and the
  function "length".) Create a class like this:

  ThinPlateSpline<SbVec3f> tps;

  This class is defined in the DD namespace. 

  \author Doug Roble
  \date March 25th, 2002 Class Creation. 
 */

#ifndef THIN_PLATE_SPLINE_H
#define THIN_PLATE_SPLINE_H

#include <vector> 

//#include <DDMath2/ArbMatrix.H>
#include "ArbMatrix.h"

namespace DD {
  namespace Math {
  
    template <class T>
    class ThinPlateSpline
    {
    public:
      /*! \brief Default constructor.
       */
      ThinPlateSpline() : a ( 1, 1 ), b ( 3, 1 ), my_min_dist ( 1e-6 )
      { /* BLEAIR b.fill(0); */ }
    
      /*! \brief Sets the constraint points for the surface. Beware, unless you
	have at least 3 non-colinear points, this puppy will not work. You must
	call this function with some points before the value function will
	work. 
      */
      bool setPoints ( std::vector<T> pnts )
      {
	constraint_pnts = pnts;
	return ( computeThinPlateCoefficients() );
      }

      /*! \brief Given a grid point and the precomputed thin plate coefficents
	and points, this function calculates the value of the surface at the
	grid point.
      */
      double value ( double x, double y ) const
      {
	double value = b ( 0, 0 ) + b ( 1, 0 ) * x + b ( 2, 0 ) * y;

	for (unsigned i = 0; i < constraint_pnts.size(); i++ )
	  {
	    T delta =
	      T ( x, y, 0 ) -
	      T ( constraint_pnts[i][0], constraint_pnts[i][1], 0 );

	    double del_len_sqr = delta[0] * delta[0] + delta[1] * delta[1];

	    if ( del_len_sqr != 0.0 )
	      value += a ( i, 0 ) * thinPlateEsqr ( del_len_sqr );
	  }
	return value; 
      } // value

      /*! \brief If points are too close together, they don't add anything to
	the solution, may cause singular matrices and make the solution take
	longer. If two points are closer than this value, one of them will be
	removed. 
       */
      double minDist () const { return my_min_dist; }

      /*! \brief Set the minimum distance between points. (The default is
	1e-6.) 
       */
      void setMinDist ( double dist ) { my_min_dist = dist; }

    protected:

      std::vector<T> constraint_pnts; //!< The set of points that the surface
      //!< will smoothly interpolate. 

      ArbDblMatrix a; //!< The weighting values for the radial basis
      //!< function. The number of rows = number of control
      //!< points. 

      ArbDblMatrix b; //!< The fixed size weights. A 3x1 matrix. 
  
      /*! \brief The distance between points that is considered "Too Damn
	Close."  One of points will be removed.
      */
      double my_min_dist; 

      /*! \brief The Radial basis function for thin plate splines. 
       */
      inline static double thinPlateE ( double r ) 
      { return r * r * log(r*r); }

      /*! \brief The Radial basis function for thin plate splines - with the
	squares removed. Make sure you send in something that has already been
	squared. 
      */
      inline static double thinPlateEsqr ( double r ) 
      { return r * log(r); }

      /*! \brief The thin plate spline is defined in terms of a radial basis
	function r * r * log ( r * r ) and some weighting parameters. Before
	we can calculate the surface, we have to solve for the weighting
	parameters such that the surface passes through the control
	points. This function does that. It should be called when ever the
	list of constraint/control points changes.
	
	\return Returns false if the solution failed (singular matrix).
      */
      bool computeThinPlateCoefficients ()
      {
	int i, j;

	int n = constraint_pnts.size();

  if (n==0)
    return false;
	// Make the arrays and make them the right size.
  
	// BLEAIR
	// a.set_size ( n, 1 );

	ArbDblMatrix A ( n, n );
	ArbDblMatrix B ( n, 3 );
	ArbDblMatrix z ( n, 1 );

	// Now, fill the A matrix. This matrix has elements
	// E_i_j = thinPlateE ( | (x_i - x_j, y_i - y_j) | )

	for ( i = 0; i < n; i++ )
	  for ( j = 0; j < n; j++ )
	    {
	      T delta =
		T ( constraint_pnts[i][0], constraint_pnts[i][1], 0 ) -
		T ( constraint_pnts[j][0], constraint_pnts[j][1], 0 );

	      double del_len_sqr = delta[0] * delta[0] + delta[1] * delta[1];

	      if ( del_len_sqr ) 
		A ( i, j ) = thinPlateEsqr ( del_len_sqr );
	      else
		A ( i, j ) = 0;
	    }

	// Fill in the B matrix. Each element is [ 1, x_i, y_i ]. The z matrix
	// is just the height of the control points.

	for ( i = 0; i < n; i++ )
	  {
	    B ( i, 0 ) = 1;
	    B ( i, 1 ) = constraint_pnts[i][0];
	    B ( i, 2 ) = constraint_pnts[i][1];

	    z ( i, 0 ) = constraint_pnts[i][2];
	  }

	// Do some inversion and transposing in prep...
	bool is_singular = false; 
	ArbDblMatrix Ainv = DD::Math::Matrix::invert (A, is_singular );

	if ( is_singular )
	  return false; 

	ArbDblMatrix Btrans = B.transpose();

	// Solve for b.

	ArbDblMatrix BAB = Btrans * Ainv * B;
	ArbDblMatrix BABinv = DD::Math::Matrix::invert(BAB, is_singular );

	b = BABinv * Btrans * Ainv * z;

	// Solve for a.
  ArbDblMatrix tmp = z - B * b;
  a = Ainv * tmp;

	return true; 
      } // computeThinPlateCoefficients

    }; // class DD::Math::ThinPlateSpline

  } // namespace Math
} // namespace DD

#endif
