/* TransformUtils.cpp
 *
 * Copyright (c) 2008 The Foundry Visionmongers Ltd.  All Rights Reserved.
 *
 * Merged from Digital Domain 11/11/08
 */

/**
 ** Mods 08-04-2008 by Raymond Yeung
 ** - Animation.h gone!
 **   Must do something ????
 ** Mods 05-27-2008 by Raymond Yeung
 ** - fixed evaluate_expression_at
 **   Subtle bug when knob can be "2" fields(knobs) collapsed into 1,
 **   therefore split_expression need to return the index of the last
 **   known channel
 **   Example:
 **      When tracker is used for "{scale {root.Tracker15.scale}}"
 **      in which scale is a single field(knob) standing in for "2".
 **      In the UI, split_expression gets only 1 channel (0) but
 **        Animation animation( knob, index );
 **        animation.from_script( channel.c_str );
 **        animation.evaluate( frame );
 **      worked passing the first channel for (1) yielding channel (0) value.
 **    * This has been reported to failed at times
 **      In command line render, this completely failed passing the first
 **      channel (0). Logic has been added in evaluate_expression_at to use
 **      the index of the last known channel from split_expression to repeat
 **      the last known value for the unknown channels.
 **
 ** Mods 03-21-2008 by Raymond Yeung
 ** - fixed 1 line in split_expression for quoted expression {\"expression\"}
 */

#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <vector>
#include <algorithm>

#include <DDImage/Iop.h>
#if DD_IMAGE_VERSION_MAJOR < 5 || \
    (DD_IMAGE_VERSION_MAJOR == 5 && DD_IMAGE_VERSION_MINOR < 1)
#include <DDImage/Animation.h>
#endif

#include "TransformUtils.h"
using namespace DD::Image;


namespace {


GridTransform default_transform;


typedef std::list< std::string > StringList;
typedef std::vector< double > DoubleList;


/**
 * \brief Evaluates a Nuke command using the given knob as the current context
 * as if it were typed on the nuke shell.
 *
 * \param knob Any valid knob.
 * \param command A Nuke command, e.g.: knob translate {0 0}
 * \param result Receives the evaluation result if successful.
 * 
 * \return True if successful. Returns false on error and leaves the result
 * parameter untouched.
 *
 * \see DD::Image::Knob::script_command DD::Image::Knob::script_result
 */
inline bool evaluate_command( Knob* knob, std::string command, std::string& result )
{
#if DD_IMAGE_VERSION_MAJOR < 5
    bool ok = knob->script_command( command.c_str() );
#else
    // turn off default python script flag for 5.0
    bool py = false;
    bool ok = knob->script_command( command.c_str(), py );
#endif
    if ( ok ) {
        result = knob->script_result();
    }
    knob->script_unlock();
    return ok;
}


/**
 * \brief Splits a valid Nuke expression into its individual channels.
 *
 * The following are the kind of valid expressions:
 * \code
 * 123
 * 123 456
 * {123}
 * {123} 456
 * {123} {456}
 * 123 {456}
 * {frame}
 * {frame*2.0}
 * {\"frame * 2.0\"}
 * {\"Tracker1.track2.x - GridWarp2_2.center.x\"}
 * {curve x1 0 x10 184 x20 314 x22 338 x30 434 x40 532}
 * \endcode
 *
 * \param expression A valid Nuke expression.
 *
 * \param number_of_channels Maximum number of channel expressions extracted
 * from the expression. If this number is higher than the actual number of
 * channels in the expression, the last channel expression is duplicated for
 * the missing requested channels. For example, the expression 123 would return
 * [ 123, 123, 123 ] if number_of_channels is set to 3.
 *
 * \return A list of strings with all the individual channel expressions.
 *
 * \todo Needs better error checking.
 */
StringList split_expression( std::string const& expression, int number_of_channels, int& last_channel_index )
{
    static StringList no_channels;

    if ( expression.empty() ) {
        return no_channels;
    }

    std::string text = expression;
    StringList channels;
    std::string last_channel;

    last_channel_index = -1;
    while ( !text.empty() ) {

        std::size_t index = std::string::npos;
        std::string channel;

        if ( text[0] != '{' ) {

            index = text.find( ' ' );
            channel = text.substr( 0, index );

        } else {

            if ( text.size() < 1 ) {
                // lartola.20080109 need a better way of reporting this error
                std::cout << "bad expression!" << std::endl;
                return no_channels;
            }

            if ( text[1] != '"' ) {

                index = text.find( '}' );
                channel = text.substr( 1, index-1 );
                index = text.find( ' ', index );

            } else {

                index = text.find( "\"}" );
                channel = text.substr( 1, index );
                index = text.find( ' ', index );

            }

        }

        if ( channel.empty() ) {
            break;
        }
        channels.push_back( channel );
        last_channel = channel;
        last_channel_index++;

        if ( index != std::string::npos ) {
            text = text.substr( index + 1 );
        } else {
            break;
        }
    }

    if ( number_of_channels > 0 ) {
        int missing_channels = number_of_channels - channels.size();
        for ( int i = 0; i < missing_channels; i++ ) {
            channels.push_back( last_channel );
        }
    }

    return channels;
}


inline void set_zero( double& value )
{
    value = 0;
}


/**
 * \brief Evaluates the Nuke expression per channel by using the given knob
 * as a bridge to the DDImage/Nuke parsing and evaluation machine.
 *
 * \param knob A valid knob used to evaluate per-channel expressions.
 * \param frame Evaluation frame.
 * \param expression Nuke expression.
 * \param values List of resulting values.
 * \param number_of_channels Evaluates up to this number of channels from
 * the expression. If set to -1, it uses the size of values to determine
 * the number of channels.
 *
 * \return True on success. False otherwise.
 */
bool evaluate_expression_at(
    Knob* knob, double frame, std::string const& expression,
    DoubleList& values, int number_of_channels = -1
)
{
    if ( !knob ) {
        return false;
    }

    if ( expression.empty() ) {
        std::for_each( values.begin(), values.end(), set_zero );
        return true;
    }

    if ( number_of_channels < 0 ) {
        number_of_channels = values.size();
    }

    int last_channel_index;
    StringList channels = split_expression( expression, number_of_channels, last_channel_index );

    int    index = 0;
    double last_value;
    StringList::const_iterator channel_it = channels.begin();
    DoubleList::iterator value_it = values.begin();

    while ( channel_it != channels.end() && value_it != values.end() ) {

        StringList::const_reference channel = *channel_it;
        DoubleList::reference value = *value_it;

        if (index <= last_channel_index) {
#if DD_IMAGE_VERSION_MAJOR < 5 || \
    (DD_IMAGE_VERSION_MAJOR == 5 && DD_IMAGE_VERSION_MINOR < 1)
            Animation animation( knob, index );
            animation.from_script( channel.c_str() );
            value = animation.evaluate( frame );
#else
	    knob->set_expression( channel.c_str(), index );
	    value = knob->get_value_at( frame, index );
#endif
	    last_value = value;
	}
        else {
	    value = last_value;
	}

        channel_it++;
        value_it++;
        index++;

    }

    knob->clear_animated();

    return true;
}


/**
 * \brief Uses the first word of a token as the key and the rest as the value.
 *
 * For example, translate 0 0, yields key = translate, value = 0 0.
 *
 * \param token A line of text.
 * \param key First word in the line.
 * \param value The rest of the line.
 */
inline void split_token( std::string const& token, std::string& key, std::string& value )
{
    std::istringstream in( token );
    in >> key;
    getline( in, value );
    std::size_t index = value.find_first_not_of( " \t\n\r" );
    if ( index != std::string::npos ) {
        value = value.substr( index );
    }
}


/**
 * \brief Sets a matrix from a string in the form {a b c d} {e f g h} {i j k l} {m n o p}.
 *
 * \param matrix A Matrix4 instance.
 * \param value Text containing a matrix usually generated when writing a Matrix4 to a
 * stream \a GridTransform::to_script.
 * \return True if succesful, false otherwise.
 */
bool matrix_from_script( Matrix4& matrix, std::string const& value )
{
    Knob::Script_List rows( value.c_str() );
    if ( rows.error() || rows.size() != 4 ) {
        std::cout << "Invalid matrix: " << value << std::endl;
        return false;
    }
    for ( int row = 0; row < 4; row++ ) {
        std::istringstream in( rows[row] );
        for ( int column = 0; column < 4; column++ ) {
            // yeap, looks weird but element access has to be transposed.
            in >> matrix[column][row];
        }
    }
    return true;
}


} // namespace




bool is_equal( Matrix4 const& left, Matrix4 const& right )
{
    float const*const a = left.array();
    float const*const b = right.array();

    for ( int i = 0; i < 16; i++ ) {
        // do as few comparisons as possible
        if ( a[i] != b[i] ) {
            return false;
        }
    }

    return true;
}





#ifdef _WIN32
// stupid windows. This function is in DDImage/Matrix4.h but IT IS NOT in
// DDImage4.8.dll. However, it is in libDDImage4.8.so on Linux.... sigh
/*! Writes it in nuke/tcl notation, with nested curly braces */
std::ostream& DD::Image::operator<< ( std::ostream &o, const DD::Image::Matrix4 &mat) {
  //  o.setf(ios::fixed, ios::floatfield);
  //o.precision(7);
  o << '{'
        << mat.a00 << " " << mat.a01 << " " << mat.a02 << " " << mat.a03 << "} {"
        << mat.a10 << " " << mat.a11 << " " << mat.a12 << " " << mat.a13 << "} {"
        << mat.a20 << " " << mat.a21 << " " << mat.a22 << " " << mat.a23 << "} {"
        << mat.a30 << " " << mat.a31 << " " << mat.a32 << " " << mat.a33 << '}';
  return o ;
}
#endif // _WIN32






GridTransform::GridTransform()
:   identity( false )
{
    make_identity();
}


void GridTransform::evaluate( Knob* knob, double frame )
{
    from_knob( knob );
    goto_frame( knob, frame );
}


void GridTransform::from_knob( Knob* knob )
{
    if ( !knob ) {
        return;
    }

    identity = false;
    evaluate_command( knob, "knob translate", translate );
    evaluate_command( knob, "knob rotate", rotate );
    evaluate_command( knob, "knob scale", scale );
    evaluate_command( knob, "knob skew", skew );
    evaluate_command( knob, "knob center", center );
}


void GridTransform::to_knob( Knob* knob ) const
{
    if ( !knob ) {
        return;
    }

    std::string result;
    evaluate_command( knob, "knob translate {" + translate + "}", result );
    evaluate_command( knob, "knob rotate {" + rotate + "}", result );
    evaluate_command( knob, "knob scale {" + scale + "}", result );
    evaluate_command( knob, "knob skew {" + skew + "}", result );
    evaluate_command( knob, "knob center {" + center + "}", result );

    // notify changes so that the actual transform jack on the
    // viewer and widgets get updated.
    knob->changed();
}


void GridTransform::goto_frame( Knob const* knob, double frame )
{
    if ( !knob ) {
        return;
    }

    Knob* evaluate = knob->knob( "evaluate" );
    if ( !evaluate ) {
        return;
    }

    DoubleList t(2), r(1), k(1), s(2), c(2);

    evaluate_expression_at( evaluate, frame, translate, t );
    evaluate_expression_at( evaluate, frame, rotate, r );
    evaluate_expression_at( evaluate, frame, skew, k );
    evaluate_expression_at( evaluate, frame, scale, s );
    evaluate_expression_at( evaluate, frame, center, c );

    matrix.translation( c[0] + t[0], c[1] + t[1] );
#if DD_IMAGE_VERSION_MAJOR < 5
    double pixel_aspect = knob->input_pixel_aspect();
#else
    // changed to Format object for input access in 5.0
    double pixel_aspect = knob->input_format().pixel_aspect();
#endif
    matrix.scale( 1.0/pixel_aspect, 1.0, 1.0 );
    matrix.rotate( r[0]*M_PI_2/90 );
    matrix.skew( k[0] );
    matrix.scale( s[0]*pixel_aspect, s[1], 1.0 );
    matrix.translate( -c[0], -c[1] );

#ifdef DEBUG_TRACE_EVALUATION
    // debugging output for sanity checks.

    std::cout << "-- expressions" << std::endl;
    std::cout << "translate| " << translate << std::endl;
    std::cout << "   rotate| " << rotate << std::endl;
    std::cout << "     skew| " << skew << std::endl;
    std::cout << "    scale| " << scale << std::endl;
    std::cout << "   center| " << center << std::endl;
    std::cout << "-- values" << std::endl;
    std::cout << "   aspect| " << pixel_aspect << std::endl;
    std::cout << "translate| " << t[0] << " " << t[1] << std::endl;
    std::cout << "   rotate| " << r[0] << std::endl;
    std::cout << "     skew| " << k[0] << std::endl;
    std::cout << "    scale| " << s[0] << " " << s[1] << std::endl;
    std::cout << "   center| " << c[0] << " " << c[1] << std::endl;
    std::cout << " computed matrix| " << matrix << std::endl;

    Matrix4 knob_matrix;
    Knob* temp_knob = const_cast< Knob* >( knob );
#if DD_IMAGE_VERSION_MAJOR < 5
    temp_knob->store( &knob_matrix, knob->frame() );
#else
    Hash hash;
    temp_knob->store( MatrixPtr, &knob_matrix, hash, knob->uiContext() );
#endif
    std::cout << "     knob matrix| " << knob_matrix << std::endl;
#endif
}


void GridTransform::make_identity()
{
    translate = "0 0";
    rotate = "0";
    scale = "1";
    skew = "0";
    center = "0 0";
    matrix.makeIdentity();
    identity = true;
}


bool GridTransform::is_identity() const
{
    return identity;
}


void GridTransform::to_script( std::ostream& out ) const
{
    out << "{";
    if ( translate != default_transform.translate ) {
        out << " {translate " << translate << "}";
    }
    if ( rotate != default_transform.rotate ) {
        out << " {rotate " << rotate << "}";
    }
    if ( scale != default_transform.scale ) {
        out << " {scale " << scale << "}";
    }
    if ( skew != default_transform.skew ) {
        out << " {skew " << skew << "}";
    }
    if ( center != default_transform.center ) {
        out << " {center " << center << "}";
    }
    if ( !is_equal( matrix, Matrix4::identity() ) ) {
        out << " {matrix " << matrix << "}";
    }
    if ( !identity ) {
        out << " {identity " << std::boolalpha << identity << std::noboolalpha << "}";
    }
    out << " }";
}


bool GridTransform::from_script( std::string const& text )
{
    Knob::Script_List actual_contents( text.c_str() );
    if ( actual_contents.error() ) {
        std::cout << "Cannot read a GridTransform from: " << text << std::endl; 
        return false;
    }
    Knob::Script_List tokens( actual_contents[0] );
    if ( tokens.error() ) {
        std::cout << "Cannot read a GridTransform from: " << text << std::endl; 
        return false;
    }

    if ( tokens.size() < 1 ) {
        make_identity();
        return true;
    }

    for ( int i = 0; i < tokens.size(); i++ ) {

        std::string key;
        std::string value;
        split_token( tokens[i], key, value );

        if ( key == "translate" ) {
            translate = value;
        } else if ( key == "rotate" ) {
            rotate = value;
        } else if ( key == "scale" ) {
            scale = value;
        } else if ( key == "skew" ) {
            skew = value;
        } else if ( key == "center" ) {
            center = value;
        } else if ( key == "matrix" ) {
            bool ok = matrix_from_script( matrix, value );
            if ( !ok ) {
                return false;
            }
        } else if ( key == "identity" ) {
            if ( key == "true" || key == "1" ) {
                identity = true;
            } else {
                identity = false;
            }
        }

    }

    return true;
}


std::ostream& operator<< ( std::ostream& out, GridTransform const& transform )
{
    transform.to_script( out );
    return out;
}



