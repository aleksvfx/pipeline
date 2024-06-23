#ifndef DD_MATH_ARB_MATRIX_H
#define DD_MATH_ARB_MATRIX_H

//#include <cmath>

#include <math.h>

#include <vector>
#include <algorithm>
#include <iostream>

#ifdef FN_VXL_MATRICES

//#include "Math/fnVecMatDefines.h"
#include "fnVecMatDefines.h"
#include "vnl/algo/vnl_svd.h"

namespace DD
{
  namespace Math
  {
    typedef Foundry::Math::Matrixd ArbDblMatrix;
    namespace Matrix
    {
      ArbDblMatrix invert(const ArbDblMatrix& input, bool& singular)
      {
        vnl_svd<double> svdobj(input);
        singular = svdobj.singularities() > 0;
        if (input.rows() != input.cols())
          return svdobj.pinverse();
        else
          return svdobj.inverse();
      }
      
      bool invert(const ArbDblMatrix& input, ArbDblMatrix& result)
      {
        bool singular;
        result = invert(input, singular);
        return singular;
      }
    }
  }
}
#else

#ifdef WIN32
//disable warnings on windows
#pragma warning( push, 1  )
#endif

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/intrusive/avltree.hpp>

#ifdef WIN32
//restore warnings state
#pragma warning( pop )
#endif


namespace DD
{
  namespace Math
  {
    /*! \file ArbMatrix.H
      The ArbMatrix class has supporting functions too. They are included at
      the bottom of the header file.
    */

    template <class ElemType, class Format, class Alloc> class ArbMatrix;

    /*! \brief  Standard definition of a matrix of doubles. */
    typedef ArbMatrix<double, boost::numeric::ublas::row_major,
                      boost::numeric::ublas::unbounded_array<double> >
    ArbDblMatrix;

    /*! \brief  Standard definition of a matrix of float. */
    typedef ArbMatrix<float, boost::numeric::ublas::row_major,
                      boost::numeric::ublas::unbounded_array<float> >
    ArbFltMatrix;

    typedef const boost::numeric::ublas::matrix_range<const ArbDblMatrix>
    ConstArbDblMatrixRange;
    typedef boost::numeric::ublas::matrix_range<ArbDblMatrix>
    ArbDblMatrixRange;

    typedef const boost::numeric::ublas::matrix_range<const ArbFltMatrix>
    ConstArbFltMatrixRange;
    typedef boost::numeric::ublas::matrix_range<ArbFltMatrix>
    ArbFltMatrixRange;

    /*! \brief ArbMatrix reimplemented as a boost sub-matrix class
    */
    template <class ElemType,
              class Format=boost::numeric::ublas::row_major,
	      class Alloc = boost::numeric::ublas::unbounded_array<ElemType> >
    class ArbMatrix :
      public boost::numeric::ublas::matrix<ElemType, Format, Alloc >
    {
      typedef ArbMatrix<ElemType, Format, Alloc> Self; 

    public:

      template<class AE>
      BOOST_UBLAS_INLINE
      ArbMatrix (const boost::numeric::ublas::matrix_expression<AE>& ae):
	boost::numeric::ublas::matrix<ElemType,Format,Alloc> ( ae )
      { }

      /*! \brief Constructor. Creates a matrix with the appropriate number of
        rows and columns and initializes the elements to a value. */
      ArbMatrix ( unsigned int num_row = 0, unsigned int num_col = 0,
		  ElemType value = 0 ) :
	boost::numeric::ublas::matrix<ElemType,Format,Alloc> ( num_row,
							       num_col )
      {
	set ( value );
      }

      /*! \brief Constructor. Creates the matrix from an array of floats. This
	is useful if you are taking data from another matrix from another
	package. SbMatrix from Inventor is one example. You, naturally, had
	better be right about the size of the array. The data is assumed to be
	stored in row-major order. All the elements of the first row are
	first. The second row are next... and so on.

	\param buffer A one dimensional array of floats. The size of this array
	should be rows * cols.
	\param rows The number of rows in the array.
	\param cols The number of columns in the array.
      */
      ArbMatrix ( const ElemType* the_buffer,
		  unsigned int rows, unsigned int cols ) :
	boost::numeric::ublas::matrix<ElemType,Format,Alloc> ( rows, cols )
      {
	std::copy ( the_buffer, the_buffer + (rows * cols), buffer() );
      } // ArbMatrix

      /*! \brief Destructor. Virtual destructor for potential subclasses. */
      virtual ~ArbMatrix () { }

      /*! \brief  Sets all the elements of the array to the same value. */
      virtual void set ( const ElemType& val )
      {
	unsigned int m, n;
	for ( m = 0; m < Self::size1(); m++ )
	  for ( n = 0; n < Self::size2(); n++ )
	    Self::operator() ( m, n ) = val;
      } // set

      /*! \brief  num_rows to preserve backwards compatability */
      inline unsigned int num_rows() const { return (unsigned int)Self::size1(); }

      /*! \brief  num_cols to preserve backwards compatability */
      inline unsigned int num_cols() const { return (unsigned int)Self::size2(); }

      /*! \brief Non-const access to an element of the array. This is legacy
        code. Use the operator() or operator[] from now on. */
      ElemType& access ( unsigned int row, unsigned int col )
      { return Self::operator() (row, col); }

      /*! \brief Const access to an element of the array. This is legacy
        code. Use the operator() or operator[] from now on. */
      ElemType access ( unsigned int row, unsigned int col ) const
      { return Self::operator() (row, col); }

      /*! \brief Sets the matrix to the identity matrix. (All zeros except for
        1s running down the diagonal. This function will work on non-square
        matrices, but the results will NOT be an identity matrix. */
      void identity ()
      {
	set ( 0 );
	unsigned int i = 0;
	while ( ( i < Self::size1() ) && ( i < Self::size2() ) )
	  {
	    Self::operator() (i, i) = 1;
	    i++;
	  }
      } // identity

      /* \brief returns access to matrix buffer(), needed to pass to other
	 people's matrix implementations. This would compile as const, but
	 since caller is free to change value leave at non const */
      ElemType *buffer() { return &(Self::data()[0]); }

      /* \brief Returns access to matrix buffer(), needed to pass to other
	 people's matrix implementations.  This would compile as const, but
	 since caller is free to change value leave at non const */
      const ElemType *buffer() const { return &(Self::data()[0]); }

      /*! Copy the current matrix to the supplied matrix. Supplied matrix
	resized if needed. */
      bool copy ( const ArbMatrix<ElemType,Format,Alloc>& mat )
      {
	if ( ( mat.num_rows() != Self::size1() ) ||
             ( mat.num_cols() != Self::size2() ) )
	  return false;

	unsigned int m,n;
	for ( m = 0; m < num_rows(); m++ )
	  for ( n = 0; n < num_cols(); n++ )
	    Self::operator()( m, n ) = mat ( m, n );
      }

      /*! \brief returns a sub matrix of this matrix, the sub matrix is from
	row r1 to r2 and column c1 to c2 INCLUSIVE might be nice to have the
	opposite of this, copying to a particular part of a matrix ? */
      const boost::numeric::ublas::matrix_range<const Self> subMatrix
      ( unsigned int r1, unsigned int r2,
	unsigned int c1, unsigned int c2 ) const
      {
	using namespace boost::numeric::ublas;
	return project
          ( *this, range ( r1, r2 + 1 ), range ( c1, c2 + 1 ) );
      }

      /*! \brief returns a sub matrix of this matrix, the sub matrix is from
	row r1 to r2 and column c1 to c2 INCLUSIVE
	non consant version, you can operate on just like a normal matrix */
      boost::numeric::ublas::matrix_range<Self> subMatrix
      ( unsigned int r1, unsigned int r2,
        unsigned int c1, unsigned int c2 )
      {
	using namespace boost::numeric::ublas;
	return project
          ( *this, range ( r1, r2 + 1 ), range ( c1, c2 + 1 ) );
      }

      /*! \brief fills up matrix with random values between 0 and max */
      void fillWithRandomValues ( ElemType max )
      {
	unsigned int i,j;

	for ( i = 0; i < Self::size1(); i++ )
	  for ( j = 0; j < Self::size2(); j++ )
	    {
	      Self::operator()(i,j) = rand()*static_cast<unsigned int>(max);
	      Self::operator()(i,j) /= RAND_MAX;
	    }

      } //fillWithRandomValues

      /*! \brief Interpolated access to any location in, or between elements of
	the array.

	If you treat the array as a big image, sometimes it's nice to figure
	out the value of an element indexed by a real number. As an example,
	if you had a 10x10 array and you wanted to know what the the value of
	the array at [5.4][3.1]? This routine implements a bilinear
	interpolation between elements to figure out this value. It doesn't
	care about bound checking. If you go off the end of the array, it'll
	interpolate correctly. */
      ElemType bilinearAccess ( double row, double col ) const
      {
	unsigned int rowi = (unsigned int) ( floor ( row ) );
	unsigned int coli = (unsigned int) ( floor ( col ) );

	ElemType sum = 0;

	unsigned int i, j;
	for ( i = 0; i <= 1; i++ )
	  {
	    // Clamp the array access to the boundaries of the array.

	    unsigned int rowc =
	      std::min ( std::max ( rowi + i, (unsigned int) 0 ),
			 ( (unsigned int) Self::size1() ) - 1 );

	    for ( j = 0; j <= 1; j++ )
	      {
		unsigned int colc =
		  std::min ( std::max ( coli + j, (unsigned int) 0),
			     ( (unsigned int) Self::size2() ) - 1 );
		sum +=
		  std::max ( (1 - fabs(row - (rowi+i))), 0.0 ) *
		  std::max ( (1 - fabs(col - (coli+j))), 0.0 ) *
		  Self::operator() ( rowc, colc );
	      }
	  }

	return sum;
      } // bilinearAccess

      /*! \brief Multiply two matrices togther. The number of columns in the
	first matrix must match the number of rows in the second. */
      ArbMatrix<ElemType,Format,Alloc> operator*
      ( const ArbMatrix<ElemType,Format,Alloc>& m ) const
      {
	ArbMatrix<ElemType,Format,Alloc> result
	  ( (unsigned int) Self::size1(), (unsigned int) m.size2() );

	matrixMult ( *this, m, result );

	return result;
      } // operator*

      /*! \brief Multiply every element in the matrix by a constant. */
      ArbMatrix<ElemType,Format,Alloc> operator* ( const ElemType& v ) const
      {
	ArbMatrix<ElemType,Format,Alloc> result ( Self::size1(), Self::size2() );
	result = boost::numeric::ublas::prod((*this),v);
	return result;
      } // operator*

      /*! \brief Return the transpose of the current matrix. */
      ArbMatrix<ElemType,Format,Alloc>
      transpose () const
      {
	ArbMatrix<ElemType,Format,Alloc> result
	  ( (unsigned int) Self::size2(), (unsigned int) Self::size1() );

	unsigned int r, c;
	for ( r = 0; r < Self::size1(); r++ )
	  for ( c = 0; c < Self::size2(); c++ )
	    result ( c, r ) = Self::operator() ( r, c );
	return result;
      } // transpose

      /*! \brief Transpose the matrix into destination matrix.  A little
	faster than above. */
      template <class Format2, class Alloc2>
      void transpose ( ArbMatrix<ElemType,Format2,Alloc2>& dest ) const
      {
	dest.resize( Self::size2(), Self::size1() );

	const ElemType* my_buffer = buffer();
	ElemType* dest_buffer;

	unsigned int r, c;
	for ( r = 0; r < Self::size1(); r++ )
	  {
	    dest_buffer = dest.buffer() + r;

	    for ( c = 0; c < Self::size2(); c++ )
	      {
		*dest_buffer = *my_buffer++;
		dest_buffer += Self::size1();
	      }
	  }
      }// transpose

      /*! \brief Computes the inverse of this matrix. This is a
	non-destructive function.

	This is a virtual function and the method of computing the inverse is
	up to the implementer. This function uses the LU decomposition
	routines which are stable and fast. This routine insists that the
	matrix is square. It will return false if the matrix was singular. (It
	still tries to invert it though.) However, because the LU
	Decomposition destroys the original matrix, a matrix copy is done.

	Subclasses \b should re-implement this function because of the temp
	copy in the method. See ArbBandedmatrix for an example.

	\param singular This is set to true if the matrix is singular. If this
	is the case, the result of this method can not be trusted!

	\return The inverse of the matrix.

	\note To make this efficient, I really should implement reference
	counting for matrices, just like for Image Bufferes in DDPattern. */
      virtual ArbMatrix<ElemType,Format,Alloc> invert ( bool& singular ) const
      {
	ArbMatrix<ElemType,Format,Alloc> working_copy = *this;
	std::vector<unsigned int> index;
	double swap_parity;

	// Check for non-square matrix. If we've got one, compute the
	// Moore-Penrose pseudoinverse.
	if ( Self::size1() != Self::size2() )
	  {
	    ArbMatrix<ElemType,Format,Alloc> square_mat
	      ( boost::numeric::ublas::prod ( working_copy.transpose(),
					      working_copy ) );

	    singular = !square_mat.LUdecomp ( index, swap_parity );

	    ArbMatrix<ElemType,Format,Alloc> I
	      ( (unsigned int) square_mat.size1(),
		(unsigned int) square_mat.size2() );
	    I.identity();

	    return square_mat.LUbacksub ( index, I ) *
	      working_copy.transpose();
	  }
	else
	  {
	    singular = !working_copy.LUdecomp ( index, swap_parity );

	    ArbMatrix<ElemType,Format,Alloc> I
	      ( (unsigned int) Self::size1(), (unsigned int) Self::size2() );
	    I.identity();

	    return working_copy.LUbacksub ( index, I );
	  }
      } // invert

      /*! \brief  \brief overload of above function, much prefer this syntax. */
      bool invert ( ArbMatrix<ElemType,Format,Alloc> &result ) const
      {
	bool singular;
	result = invert(singular);
	return singular;
      } // invert

      /*! \brief Assignment statement. This method will resize the matrix if
	neccessary.

	this is needed because of complications with the boost ublas library,
	it needs to make a copy of the right hand side of the equation
	A = general matrix function
	because it does not know if the rhs function references A you can get
	around this by using alias but this is ugly we specialize the function
	to work with arb matrices so that arb matrices to arb matrix copying
	doesn't do a copy every time */
      ArbMatrix<ElemType,Format,Alloc> & operator=
	( const ArbMatrix<ElemType,Format,Alloc> & m )
      {
	if ( this == &m ) return *this;

	if ( ( num_rows() != m.num_rows() ) || ( num_cols() != m.num_cols() ) )
          Self::resize( m.num_rows(), m.num_cols() );

	assign (m);

	return *this;
      }

      /*! \brief Assignment statement. This method will resize the matrix if
	neccessary.
	Allows different template parameters for the rhs.
	\note Do we need the above one and this one? */
      template <class ElemType_2, class Format_2, class Alloc_2>
      ArbMatrix<ElemType,Format,Alloc>& operator=
	( const ArbMatrix<ElemType_2,Format_2,Alloc_2> & m )
      {
	  if ( this == &m ) return *this;

	  if ( (num_rows() != m.num_rows()) || (num_cols() != m.num_cols()) )
	    Self::resize ( m.num_rows(), m.num_cols() );

	  assign (m);
	  return *this;
	}

      /*! \brief This figures out the determinant of the matrix by doing a LU
	decomposition and multiplying the diagonals.
        \warning For any kind of size, this could overflow even a double,
        beware. */
      virtual ElemType determinant ()
      {
	ArbMatrix<ElemType,Format,Alloc> Temp ( *this );
	std::vector<unsigned int> index;
	double dtot;
	Temp.LUdecomp ( index, dtot );
	unsigned int j;
	for ( j = 0; j < Temp.size1(); j++ ) dtot *= Temp ( j, j );
	return dtot;
      } // determinant

      /*! \brief Calculates the eigenvalues (not eigenvectors) of a 2x2
	matrix.

	This will eventually be replaced with a true eigenvalue calculator, but
	I need only a 2x2 matrix calculator now, and that is relatively easy.

	\return A vector of ElemTypes, each representing the real (non-complex)
	eigenvectors of this matrix. There can be at most two entries.

	\warning Only valid for 2x2 matrices. Any other matrix will return a
	zero-length vector. */
      virtual std::vector<ElemType> eigenvalues () const
      {
	std::vector<ElemType> evalues;
	if ( ( Self::size1() == 2 ) && ( Self::size2() == 2 ) )
	  {
	    ElemType diag_sum = Self::operator() ( 0, 0 ) + Self::operator() ( 1, 1 );
	    ElemType under_sqrt = diag_sum * diag_sum -
	      4 * ( Self::operator() ( 0, 0 ) * Self::operator() ( 1, 1 ) -
		    Self::operator() ( 0, 1 ) * Self::operator() ( 1, 0 ) );
	    if ( under_sqrt >= 0 )
	      {
		ElemType the_sqrt = sqrt ( under_sqrt );
		evalues.push_back ( ( diag_sum + the_sqrt ) / 2 );
		evalues.push_back ( ( diag_sum - the_sqrt ) / 2 );
	      }
	  }
	return evalues;
      } // eigenvalues

      /*! \brief Replaces the value of this matrix with the LU Decomposition
	of the matrix. This is based on Crout's algorithm with partial
	pivoting. It can be found in any good linear algebra book (or
	Numerical Recipes in C). This is lovely if you wish to solve a linear
	system of equations: A x = b.

	\param index This records the the row swaping that has been done. It's
	a list of row indices in the way the decomposition was forced to put
	them.

	\param swap_parity This variable is set to +1 if the number of row
	swaps was even, and -1 if the swaps were odd.

	\return If the decomposition was successful, the function will return
	true. If there was a problem - the matrix was singular or it wasn't a
	square matrix, it will return false. */
      virtual bool LUdecomp ( std::vector<unsigned int>& index,
			      double& swap_parity )
      {
	if ( Self::size1() != Self::size2() ) return false; // Matrix must be square.

	// Reserve some space.

	// Implicit row scaling, initialized to 1.0
	std::vector<double> row_scaling ( Self::size1(), 1.0 );

	index = std::vector<unsigned int> ( Self::size1(), 0 );
	swap_parity = 1.0;  // Init to 1.0, no swaps yet.
	unsigned int i, j, k;
	bool singular = false;

	// Examine each row to get the scaling information.

	for ( i = 0; i < Self::size1(); i++ )
	  {
	    ElemType big = 0.0;
	    for ( j = 0; j < Self::size2(); j++ )
	      big = std::max ( big, (ElemType)fabs ( Self::operator() ( i, j ) ) );

	    if ( big == 0.0 ) return false; // All zero row == singular matrix.

	    row_scaling[i] = 1.0 / big;
	  }

	// Loop over each column.

	for ( j = 0; j < Self::size2(); j++ )
	  {
	    // Compute beta[i][j].

	    for ( i = 0; i < j; i++ )
	      {
		ElemType sum = Self::operator() ( i, j );
		for ( k = 0; k < i; k++ )
		  sum -= Self::operator() ( i, k ) * Self::operator() ( k, j );
		Self::operator() ( i, j ) = sum;
	      }

	    // Calculate sum for row j and remember it:
	    ElemType big = Self::operator() ( j, j );
	    for ( k = 0; k < j; k++ )
	      big -= Self::operator() ( j, k ) * Self::operator() ( k, j );
	    Self::operator() ( j, j ) = big;
	    big = row_scaling[j] * fabs ( big );

	    // Now, the next part of beta[i][j]...
	    unsigned int best_row = j;
	    for ( i = j+1; i < Self::size1(); i++ )
	      {
		ElemType sum = Self::operator() ( i, j );
		for ( k = 0; k < j; k++ )
		  sum -= Self::operator() ( i, k ) * Self::operator() ( k, j );
		Self::operator() ( i, j ) = sum;

		// Is this the best pivot?

		ElemType temp = row_scaling[i] * fabs ( sum );
		if ( temp >= big )
		  {
		    big = temp;
		    best_row = i;
		  }
	      }

	    // Check to see if we need to exchange rows.

	    if ( j != best_row )
	      {
		for ( k = 0; k < Self::size2(); k++ )
		  std::swap ( Self::operator()( best_row, k ), Self::operator()( j, k ) );
		swap_parity = -swap_parity;  // record that we did a swap.
		row_scaling[best_row] = row_scaling[j];
	      }

	    index[j] = best_row; // Save the index.

	    // If the pivot element is zero, the matrix is singular. Numerical
	    // Recipes suggests substituting a small value so that some
	    // applications might recover.

	    if ( Self::operator() ( j, j ) == 0.0 )
	      {
		Self::operator() ( j, j ) = 1.0e-20;
		singular = true;
	      }

	    // Divide by the pivot element.

	    if ( j != Self::size2() - 1 )
	      {
		ElemType pivot = 1.0 / Self::operator() ( j, j );
		for ( i = j + 1; i < Self::size1(); i++ )
		  Self::operator()( i, j ) *= pivot;
	      }
	  } // End of one column.

	return !singular;
      } // LUdecomp

      /*! \brief This is the companion member function to LUdecomp. Once
	you've done the decomposition, you call this function to solve for x
	in the equation A x = b.

	If the matrix is singular or the size of the b vector doesn't match,
	the results are undefined.

	\param index This is the swapping permutation index array from the
	decomposition routine.

	\param b This can be either a vector or a matrix. The number of rows
	must be equal to the number of rows of A. If the number of columns is
	greater than one, the back substitution will be run for each column,
	producing a multicolumn resultant vector.

	\return This method, unlike some other implementations, doesn't
	destroy the b vector(matrix). The solution for the x vector(matrix) is
	the return value of this method. The size of the result is the same as
	the b input.
      */
      virtual ArbMatrix<ElemType,Format,Alloc> LUbacksub
      ( std::vector<unsigned int>& index,
	const ArbMatrix<ElemType,Format,Alloc>& b )
      {
	ArbMatrix<ElemType,Format,Alloc> x = b;
	if ( b.size1() != Self::size2() ) return x;

	ElemType sum;

	// Do the back substitution for each column in b (or x);
	for (unsigned k = 0; k < x.size2(); k++ )
	  {
	    // Forward substitution first.

	    int inonzero = -1;  // Index of first non-zero b entry.
	    unsigned i;
	    for ( i = 0; i < Self::size1(); i++ )
	      {
		unsigned int ip = index[i];

		sum = x(ip, k);
		x(ip, k) = x(i, k);

		if ( inonzero != -1 )
		  for (unsigned j = (unsigned)inonzero; j < i; j++ )
		    sum -= Self::operator() ( i, j ) * x(j, k);
		else if ( sum )
		  inonzero = (int)i;

		x(i, k) = sum;
	      }

	    // Back substitution.

	    for ( i = Self::size1(); i--; )
	      {
		sum = x.operator()(i,k);
		for (unsigned j = i + 1; j < Self::size1(); j++ )
		  sum -= Self::operator()( i, j ) * x.operator()(j,k);
                x.operator()(i,k) = sum / Self::operator() ( i, i );
	      }
	  }

	return x;
      } // LUbacksub

      /*!  \brief Cholesky decomposition scheme for symmetric, positive
	definite matrices.  The contents of the current matrix are
	replaced with the L factor in the lower triangle.  This method can
	also be used to check if the matrix is positive definite since
	it's so simple and efficient.

	\warning Does no pivoting, however this technique is very
	numerically stable without pivoting.

	\warning LU decomposition works for SPD matrices, this is just
	a bit more efficient.

	\return Boolean indicating success or failure of this method.
	False implies that the matrix was not a positive definite matrix.
      */
      bool CholeskyDecomp () {
	unsigned int i, j, k;
	ElemType sum = 0.;
	bool spd = true;

	// we're only interested in the upper triangle

	/*!
	  simple explanation is we compute L of LU.  And replace the
	  diagonals w/ the square root of the diagonals of U
	*/
	for (i = 0; i < Self::size1(); i++) { //iterate rows
	  for (j = i; j < Self::size2(); j++) { //iterate columns
	    for (k = i - 1, sum = Self::operator() ( i, j ); k >= 0; k--)
	      sum -= Self::operator() ( i, k ) * Self::operator() ( j, k );
	    if (i == j) {
	      if (sum <= 0.)
		return (!spd);
	      Self::operator() ( i, j ) = sqrt(sum);
	    }
	    else
	      Self::operator() ( j, i ) = sum / Self::operator() ( i, i );
	  } //end iter cols
	}//end iter rows

	return (spd);
      } // CholeskyDecomp

      /*! \brief converts an sbMatrix to this an ArbMatrix.

      transpose flag means the matrix get transposed as it's being copied
      */
      template <class SbMatrixType>
      ArbMatrix<ElemType,Format,Alloc>& fromSbMatrix
      ( const SbMatrixType& sb_matrix, bool transpose = false )
      {
        Self::resize(4,4);

	unsigned int i,j;
	if  (transpose)
	  {
	    for ( i = 0; i < Self::size1(); i++ )
	      for ( j = 0; j < Self::size2(); j++ )
		Self::operator()(j,i) = sb_matrix[i][j];
	  }
	else
	  {
	    for ( i = 0; i < Self::size1(); i++ )
	      for ( j = 0; j < Self::size2(); j++ )
		Self::operator()(i,j) = sb_matrix[i][j];
	  }

	return (*this);

      }//getSbMatrix

      /*! \brief converts this matrix to an invector sbMatrix
	transpose flag means the matrix get transposed as it's being copied
      */
      template <class SbMatrixType>
      SbMatrixType getSbMatrix ( bool transpose = false ) const
      {
	SbMatrixType sb_matrix;
	if (Self::size2() != 4 || Self::size1() != 4)
	  {
	    std::cerr <<
	      "DD::Math::ArbMatrix::getSbMatrix rows and cols must be 4" <<
	      std::endl;
	    sb_matrix.makeIdentity();
	  }
	else
	  {
	    unsigned int i,j;
	    if ( transpose )
	      {
		for ( i = 0; i < Self::size1(); i++ )
		  for ( j = 0; j < Self::size2(); j++ )
		    sb_matrix[j][i] = Self::operator()(i,j);
	      }
	    else
	      {
		for ( i = 0; i < Self::size1(); i++ )
		  for ( j = 0; j < Self::size2(); j++ )
		    sb_matrix[i][j] = Self::operator()(i,j);
	      }
	}
	return sb_matrix;

      } //getSbMatrix

      /*! \brief Solves for x in Ax = b for a symmetric, positive definite
	matrix A.  This should be used to solve this system once
	CholeskyDecomp() has been run on A.

	\return The solution to Ax=b, ie x.  b is not overwritten.
      */
      ArbMatrix<ElemType,Format,Alloc> CholeskyBacksub
      ( const ArbMatrix<ElemType,Format,Alloc>& b )
      {
	int i, k;
	ElemType sum;
	ArbMatrix<ElemType,Format,Alloc> x = b;

	/*
	  In context of cholesky decomp we solve Ax = b for x by:
	  1. solving Ly = b for y
	  2. solving L^T x = y for x
	*/

	for ( i = 0; i < Self::size1(); i++ ) { // soln for Ly = b
	  for ( sum = b[0][i], k = i - 1; k >= 0; k-- )
	    sum -= Self::operator() ( i, k ) * x[0][k];

	  x[0][i] = sum / Self::operator() ( i, i );
	}

	for ( i = Self::size2() - 1; i >= 0; i-- ) { // soln for L^T x = y
	  for ( sum = x[0][i], k = i + 1; k < Self::size2(); k++ )
	    sum -= Self::operator() ( k, i ) * x[0][k];
	  x[0][i] = sum / Self::operator() ( i, i );
	}

	return x;
      } // CholeskyBacksub

      /*! \brief A tridiagonal matrix is a special kind of banded matrix with
	non-zero entries only along the main diagonal and the diagonals above
	and below this. These kinds of matrices are terribly easy to solve as
	linear systems. This method solves for x in A * x = b if A is
	tridiagonal.

	\note This tridiagonal solver will work on any kind of matrix, but you
	really should use it with a banded matrix to save space.

	\note Also, this sucker will not work at all if the matrix is
	singular.  Beware.

	\return The n dimensional matrix representing the solution for x for
	each column of b.
      */
      ArbMatrix<ElemType,Format,Alloc> tridiagonal
      ( const ArbMatrix<ElemType,Format,Alloc>& b, bool& success )
      {
	success = false;  // Assume the worst!
	ArbMatrix<ElemType,Format,Alloc> x = b;
	if ( b.size1() != Self::size1() ) {
	  std::cerr << "ArbMatrix::tridiagonal: Matrix size mismatch error."
		    << std::endl;
	  return x;
	}

        int j, k;
	unsigned int n = b.size1();
	std::vector<ElemType> gamma ( n );  // A workspace.

	if ( Self::operator() ( 0, 0 ) == 0.0 ) {
	  std::cerr << "ArbMatrix::tridiagonal: First element zero error."
		    << std::endl;
	  return x;
	}

	// Solve for each column in b (or x).

	for ( k = 0; k < x.size2(); k++ )
	  {
	    ElemType beta = Self::operator() ( 0, 0 );
	    x[0][k] = b[0][k] / beta;
	    for ( j = 1; j < n; j++ )
	      {
		gamma[j] = Self::operator() ( j-1, j ) / beta;
		beta = Self::operator() ( j, j ) -
		  Self::operator() ( j, j-1 ) * gamma[j];
		if ( beta == 0.0 ) {
		  std::cerr << "ArbMatrix::tridiagonal:"
		    " Unstable/singular matrix error." << std::endl;
		  return x;
		}

		x[j][k] =
		  ( b[j][k] - Self::operator()( j, j-1 ) * x[j-1][k] ) / beta;
	      }

	    // Back subtitution.
	    for ( j = ( n - 2 ); j >= 0; j-- )
	      x[j][k] -= gamma[j+1] * x[j+1][k];
	  }

	success = true;
	return x;
      } // tridiagonal


    }; // class ArbMatrix

    /*! \brief This function multiplies two matrices together and puts the
      result in another matrix. ( A * B = result ) You should use this
      function if you already have the result matrix to avoid copying
      matrices. The operator* is nice for chaining things together.

      \param A An M x N matrix.
      \param B An N x P matrix.
      \param result An M x P matrix. If the size of the matrix is not correct,
      this function will resize it.
    */
    template <class ElemType,class Format,class Alloc>
    inline void matrixMult ( const ArbMatrix<ElemType,Format,Alloc>& A,
			     const ArbMatrix<ElemType,Format,Alloc>& B,
			     ArbMatrix<ElemType,Format,Alloc>& result )
    {
      if ( A.size2() != B.size1() )
	{
	  std::cerr << "DD::Math::ArbMatrix matrix multiply: Error. Attempt "
		    << "to multiply matrices with non-matching cols/rows."
		    << std::endl;
	  std::cerr << "A Matrix " << (unsigned int)A.size1()
		    << " x " << (unsigned int)A.size2() << std::endl;
	  std::cerr << "B Matrix " << (unsigned int)B.size1()
		    << " x " << (unsigned int)B.size2() << std::endl;
	  return;
	}

      if ( ( result.size1() != A.size1() ) ||
	   ( result.size2() != B.size2() ) )
	result.resize ( A.size1(), B.size2() );

      noalias(result) = boost::numeric::ublas::prod(A,B);
    } // matrixMult

    /*! \brief Output operator. This writes out the matrix in a nice, easy to
      read form. */
    template <class ElemType,class Format,class Alloc>
    inline std::ostream& operator<<
      ( std::ostream &o, const ArbMatrix<ElemType,Format,Alloc> &m )
    {
      o << "Array size: " <<(unsigned int) m.size1() << " rows X "
	<<(unsigned int) m.size2() << " columns." << std::endl;

      unsigned int r, c;
      for ( r = 0; r < m.size1(); r++ )
	{
	  o << "[ ";

	  for ( c = 0; c < m.size2(); c++ )
	    {
	      ElemType val = m ( r, c );
	      unsigned int ival = int(val);
	      ElemType rem = fabs ( val - ival );
	      if ( rem < 1e-8 )
		o << "[ " << ival << " ] ";
	      else
		o << "[ " << val << " ] ";
	    }

	  o	<< "]" << std::endl;
	}
      return o;
    } // operator <<

  } // namespace Math
} // namespace DD
#endif
#endif //FN_VXL_MATRICES
