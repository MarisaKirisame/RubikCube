#ifndef CUBE_HPP
#define CUBE_HPP
#include <thread>
#include <iostream>
#include <memory>
#include <array>
#include <algorithm>
#include <utility>
#include <iterator>
#include <atomic>
#include "../Artificial_Intelligence_A_Modern_Approach/search.hpp"
#include <../misc/iterator.hpp>
enum class color { red, green, blue, yellow, orange, white, wildcard };
constexpr size_t cube_length = 3;/*Change rotate and position
                                  *if you want to change this*/
template< typename OS >
OS & operator << ( OS & os, color c )
{
    return os <<
        (c == color::red ? 'r' :
            c == color::blue ? 'b' :
                c == color::yellow ? 'y' :
                    c == color::green ? 'g' :
                        c == color::orange ? 'o' : 'w');
}

template< typename T >
void swap( T & t1, T & t2, T & t3, T & t4 ) noexcept(noexcept(std::swap(t1, t2)))
{
    T tmp( std::move( t1 ) );
    t1 = std::move( t2 );
    t2 = std::move( t3 );
    t3 = std::move( t4 );
    t4 = std::move( tmp );
}

template< typename color >
struct faces
{
    std::array< std::array< color, cube_length >, cube_length > data;
    //[0][0], [0][1], [0][2]
    //[1][0], [1][1], [1][2]
    //[2][0], [2][1], [2][2]
    faces( color c )
    {
        auto r = misc::make_iterator_iterator( data.begin( ), data.end( ) );
        std::for_each( r.first, r.second, [c](auto & mc){ mc = c; } );
    }
    std::array< color, cube_length > & operator[]( size_t s ) { return data[s]; }
    const std::array< color, cube_length > & operator[]( size_t s ) const { return data[s]; }
    void rotate_counter_clockwise( )
    {
        swap( data[0][0], data[0][2], data[2][2], data[2][0] );
        swap( data[0][1], data[1][2], data[2][1], data[1][0] );
    }
    void flip( )
    {
        rotate_counter_clockwise( );
        rotate_counter_clockwise( );
    }
    void rotate_clockwise( )
    {
        rotate_counter_clockwise( );
        rotate_counter_clockwise( );
        rotate_counter_clockwise( );
    }
    bool operator == ( const faces & cmp ) const { return data == cmp.data; }
    bool operator < ( const faces & cmp ) const { return data < cmp.data; }
    bool is_fix( const color & original_color ) const
    {
        auto ran = make_iterator_iterator( data.begin( ), data.end( ) );
        return std::all_of(
                ran.first,
                ran.second,
                [original_color]( const color & c ) { return c == original_color; } );
    }
};

struct move
{
    enum { left, right, front, back, up, down, middle, equator, standing } position;
    enum { clockwise, counter_clockwise, flip } degree;
};

struct position
{
    enum { left, right, front, back, up, down } face;
    std::pair< size_t, size_t > coordinate;
};

template< typename OS >
OS & operator << ( OS & os, const move & m )
{
    return os << (
        m.position == move::left ? 'L' :
        m.position == move::right ? 'R' :
        m.position == move::front ? 'F' :
        m.position == move::back ? 'B' :
        m.position == move::up ? 'U' :
        m.position == move::down ? 'D' :
        m.position == move::middle ? 'M' :
        m.position == move::equator ? 'E' : 'S' ) << (
            m.degree == move::clockwise ? ' ' :
            m.degree == move::counter_clockwise ? '\'' : '2' );
}

template< typename color >
struct cube
{
    faces< color > front, back, left, right, up, down;

    template< typename IT >
    void make_moves( IT begin, IT end )
    { std::for_each( begin, end, [this]( const move & m ) { make_move( m ); } ); }

    template< typename IT >
    void undo_moves( IT begin, IT end )
    { std::for_each( begin, end, [this]( const move & m ) { undo_move( m ); } ); }

    void undo_move( const move & m )
    {
        make_move( m );
        make_move( m );
        make_move( m );
    }

    void make_move( const move & m )
    {
        size_t repeat = m.degree == m.clockwise ? 1 : m.degree == m.flip ? 2 : 3;
        while ( repeat != 0 )
        {
            --repeat;
            switch ( m.position )
            {
                case move::front:
                    front.rotate_clockwise( );
                    for( size_t i = 0; i < cube_length; ++i )
                    {
                        swap(
                            up[cube_length - 1][i],
                            left[cube_length - 1 - i][cube_length - 1],
                            down[0][cube_length - 1 - i],
                            right[i][0] );
                    }
                    break;
                case move::back:
                    back.rotate_clockwise( );
                    for( size_t i = 0; i < cube_length; ++i )
                    {
                        swap(
                            up[0][i],
                            right[i][cube_length - 1],
                            down[cube_length - 1][cube_length - 1 - i],
                            left[cube_length - 1 - i][0] );
                    }
                    break;
                case move::left:
                    left.rotate_clockwise( );
                    for( size_t i = 0; i < cube_length; ++i )
                    {
                        swap(
                            down[i][0],
                            front[i][0],
                            up[i][0],
                            back[i][0] );
                    }
                    break;
                case move::right:
                    right.rotate_clockwise( );
                    for( size_t i = 0; i < cube_length; ++i )
                    {
                        swap(
                            back[i][cube_length - 1],
                            up[i][cube_length - 1],
                            front[i][cube_length - 1],
                            down[i][cube_length - 1] );
                    }
                    break;
                case move::up:
                    up.rotate_clockwise( );
                    for( size_t i = 0; i < cube_length; ++i )
                    {
                        swap(
                            back[cube_length - 1][i],
                            left[0][cube_length - 1 - i],
                            front[0][cube_length - 1 - i],
                            right[0][cube_length - 1 - i] );
                    }
                    break;
                case move::down:
                    down.rotate_clockwise( );
                    for( size_t i = 0; i < cube_length; ++i )
                    {
                        swap(
                            right[cube_length - 1][i],
                            front[cube_length - 1][i],
                            left[cube_length - 1][i],
                            back[0][cube_length - 1 - i] );
                    }
                    break;
                case move::middle:
                    for( size_t i = 0; i < cube_length; ++i )
                    {
                        swap(
                            down[i][1],
                            front[i][1],
                            up[i][1],
                            back[i][1] );
                    }
                    break;
                case move::equator:
                    for( size_t i = 0; i < cube_length; ++i )
                    {
                        swap(
                            right[1][i],
                            front[1][i],
                            left[1][i],
                            back[1][cube_length - 1 - i] );
                    }
                    break;
                case move::standing:
                    for( size_t i = 0; i < cube_length; ++i )
                    {
                        swap(
                            up[1][i],
                            left[cube_length - 1 - i][1],
                            down[1][cube_length - 1 - i],
                            right[i][1] );
                    }
                    break;
            }
        }
    }
    bool is_fix( const color & front_color, const color & back_color, const color & left_color,
                 const color & right_color, const color & up_color, const color & down_color ) const
    {
        return
            front.is_fix( front_color ) && back.is_fix( back_color ) && left.is_fix( left_color ) &&
            right.is_fix( right_color ) && up.is_fix( up_color ) && down.is_fix( down_color );
    }
    auto tie( ) const { return std::tie( front, back, left, right, up, down ); }
    bool operator < ( const cube & cmp ) const { return tie( ) < cmp.tie( ); }
    bool operator == ( const cube & cmp ) const { return tie( ) == cmp.tie( ); }
    cube( faces< color > && front, faces< color > && back, faces< color > && left,
          faces< color > && right, faces< color > && up, faces< color > && down ) :
        front( std::move( front ) ), back( std::move( back ) ), left( std::move( left ) ),
        right( std::move( right ) ), up( std::move( up ) ), down( std::move( down ) ) { }
    template< typename OS >
    friend OS & operator << ( OS & os, const cube & c )
    {
        for ( size_t i = 0; i < cube_length; ++i )
        {
            os <<
                std::string( cube_length, ' ' ) <<
                c.back[i][0] <<
                c.back[i][1] <<
                c.back[i][2] << std::endl;
        }
        for ( size_t i = 0; i < cube_length; ++i )
        {
            os <<
                std::string( cube_length, ' ' ) <<
                c.up[i][0] <<
                c.up[i][1] <<
                c.up[i][2] << std::endl;
        }
        for ( size_t i = 0; i < cube_length; ++i )
        {
            os <<
                c.left[i][0] <<
                c.left[i][1] <<
                c.left[i][2] <<
                c.front[i][0] <<
                c.front[i][1] <<
                c.front[i][2] <<
                c.right[i][0] <<
                c.right[i][1] <<
                c.right[i][2] << std::endl;
        }
        for ( size_t i = 0; i < cube_length; ++i )
        {
            os <<
                std::string( cube_length, ' ' ) <<
                c.down[i][0] <<
                c.down[i][1] <<
                c.down[i][2] << std::endl;
        }
        return os;
    }
};

const std::vector< move > & all_moves( )
{
    static const std::vector< move > ret(
        []( )
        {
            std::vector< move > inner;
            auto f =
                [&inner]( auto wf )
                {
                    inner.push_back( move { wf, move::clockwise } );
                    inner.push_back( move { wf, move::counter_clockwise } );
                    inner.push_back( move { wf, move::flip } );
                };
            f( move::back );
            f( move::front );
            f( move::up );
            f( move::down );
            f( move::left );
            f( move::right );
            f( move::equator );
            f( move::standing );
            f( move::middle );
            return inner;
        }( ) );
    return ret;
}
#endif // CUBE_HPP
