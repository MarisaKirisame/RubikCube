#include <thread>
#include <iostream>
#include <memory>
#include <array>
#include <algorithm>
#include <utility>
#include <iterator>
#include <atomic>
#include "../Artificial_Intelligence_A_Modern_Approach/search.hpp"
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

template< typename IT >
struct range_container_proxy
{
    IT b, e;
    IT begin( ) const { return b; }
    IT end( ) const { return e; }
    range_container_proxy( IT b, IT e ) : b( b ), e( e ) { }
};

template< typename IT >
range_container_proxy< IT > make_range_container_proxy( IT begin, IT end )
{ return range_container_proxy< IT >( begin, end ); }

template< typename IT >
struct iterator_iterator
{
    IT current, end;
    using IIT = decltype( std::declval< IT >( )->begin( ) );
    IIT icurrent, iend;
    iterator_iterator & operator ++( )
    {
        ++icurrent;
        while ( icurrent == iend )
        {
            if ( current == end ) { return * this; }
            ++current;
            icurrent = current->begin( );
            iend = current->end( );
        }
        return * this;
    }
    using element = decltype(* icurrent);
    const element & operator * ( ) const { return * icurrent; }
    element & operator * ( ) { return * icurrent; }
    template< typename T >
    iterator_iterator( const T & c, const T & e ) : current( c ), end( e )
    {
        if ( current != end )
        {
            icurrent = current->begin( );
            iend = current->end( );
        }
        while ( icurrent == iend )
        {
            if ( current == end ) { return; }
            icurrent = current->begin( );
            iend = current->end( );
            ++current;
        }
    }
    bool operator ==( const iterator_iterator & cmp ) const
    { return current == cmp.current && end == cmp.end; }
    bool operator !=( const iterator_iterator & cmp ) const
    { return ! ((* this) == cmp); }
};

template< typename IT >
struct std::iterator_traits< iterator_iterator< IT > >
{
    using iterator_category = std::forward_iterator_tag;
    using reference = typename std::iterator_traits< typename iterator_iterator< IT >::IIT >::reference;
    using pointer = typename std::iterator_traits< typename iterator_iterator< IT >::IIT >::pointer;
    using value_type = typename std::iterator_traits< typename iterator_iterator< IT >::IIT >::value_type;
};

template< typename IT >
struct std::iterator_traits< range_container_proxy< IT > > : std::iterator_traits< IT > { };

template< typename T >
std::pair< iterator_iterator< T >, iterator_iterator< T > >
make_iterator_iterator( const T & b, const T & e )
{
    using ret_type = iterator_iterator< T >;
    return std::make_pair( ret_type( b, e ), ret_type( e, e ) );
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
        auto r = make_iterator_iterator( data.begin( ), data.end( ) );
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

int main( )
{
    cube< color > c(
        faces< color > { color::blue },
        faces< color > { color::green },
        faces< color > { color::orange },
        faces< color > { color::red },
        faces< color > { color::white },
        faces< color > { color::yellow } );
    std::vector< move > moves =
    {
        { move::up, move::clockwise },
        { move::right, move::flip },
        { move::front, move::clockwise },
        { move::back, move::clockwise },
        { move::right, move::clockwise },
        { move::back, move::flip },
        { move::right, move::clockwise },
        { move::up, move::flip },
        { move::left, move::clockwise },
        { move::back, move::flip },
        { move::right, move::clockwise },
        { move::up, move::counter_clockwise },
        { move::down, move::counter_clockwise },
        { move::right, move::flip },
        { move::front, move::clockwise },
        { move::right, move::counter_clockwise },
        { move::left, move::clockwise },
        { move::back, move::flip },
        { move::up, move::flip },
        { move::front, move::flip }
    };
    c.make_moves( moves.begin( ), moves.end( ) );
    using wild_cube = cube< std::set< color > >;
    auto make_wild_cube = []( ) -> wild_cube
    {
        return
        {
            faces< std::set< color > > { { } },
            faces< std::set< color > > { { } },
            faces< std::set< color > > { { } },
            faces< std::set< color > > { { } },
            faces< std::set< color > > { { } },
            faces< std::set< color > > { { } }
        };
    };
    std::list< wild_cube > middle_steps =
    {
        make_wild_cube( ),
        make_wild_cube( ),
        make_wild_cube( ),
        make_wild_cube( ),
        make_wild_cube( ),
        make_wild_cube( ),
        make_wild_cube( ),
        make_wild_cube( ),
        make_wild_cube( ),
        make_wild_cube( ),
        make_wild_cube( ),
        make_wild_cube( ),
        make_wild_cube( ),
        make_wild_cube( ),
        make_wild_cube( )
    };
    auto it = middle_steps.begin( ), end = middle_steps.end( );
    std::for_each(
        it,
        end,
        [&]( wild_cube & wc )
        {
            wc.front[1][1] =
            { c.front[1][1] };
            wc.back[1][1] =
            { c.back[1][1] };
            wc.left[1][1] =
            { c.left[1][1] };
            wc.right[1][1] =
            { c.right[1][1] };
            wc.up[1][1] =
            { c.up[1][1] };
            wc.down[1][1] =
            { c.down[1][1] };
        } );
    ++it;
    std::for_each(
        it,
        end,
        [&]( wild_cube & wc )
        {
            wc.down[0][0] =
            wc.down[0][2] =
            wc.down[2][0] =
            wc.down[2][2] =
            { c.down[1][1] };
            wc.front[2][0] =
            wc.front[2][2] =
            { c.front[1][1] };
            wc.left[2][0] =
            wc.left[2][2] =
            { c.left[1][1] };
            wc.right[2][0] =
            wc.right[2][2] =
            { c.right[1][1] };
            wc.back[0][0] =
            wc.back[0][2] =
            { c.back[1][1] };
        } );
    ++it;
    std::for_each(
        it,
        end,
        [&]( wild_cube & wc )
        {
            wc.up[0][0] =
            wc.up[0][2] =
            wc.up[2][0] =
            wc.up[2][2] =
            { c.up[1][1] };
            wc.front[0][0] =
            wc.front[0][2] =
            { c.front[1][1] };
            wc.left[0][0] =
            wc.left[0][2] =
            { c.left[1][1] };
            wc.right[0][0] =
            wc.right[0][2] =
            { c.right[1][1] };
            wc.back[2][0] =
            wc.back[2][2] =
            { c.back[1][1] };
        } );
    ++it;
    std::for_each(
        it,
        end,
        [&]( wild_cube & wc )
        {
            wc.left[1][2] =
            { c.left[1][1] };
            wc.front[1][0] =
            { c.front[1][1] };
        } );
    ++it;
    std::for_each(
        it,
        end,
        [&]( wild_cube & wc )
        {
            wc.left[2][1] =
            { c.left[1][1] };
            wc.down[1][0] =
            { c.down[1][1] };
        } );
    ++it;
    std::for_each(
        it,
        end,
        [&]( wild_cube & wc )
        {
            wc.left[1][0] =
            { c.left[1][1] };
            wc.back[1][0] =
            { c.back[1][1] };
        } );
    ++it;
    std::for_each(
        it,
        end,
        [&]( wild_cube & wc )
        {
            wc.right[1][0] =
            { c.right[1][1] };
            wc.front[1][2] =
            { c.front[1][1] };
        } );
    ++it;
    std::for_each(
        it,
        end,
        [&]( wild_cube & wc )
        {
            wc.right[0][1] =
            { c.right[1][1] };
            wc.up[1][2] =
            { c.up[1][1] };
        } );
    ++it;
    std::for_each(
        it,
        end,
        [&]( wild_cube & wc )
        {
            wc.right[2][1] =
            { c.right[1][1] };
            wc.down[1][2] =
            { c.down[1][1] };
        } );
    ++it;
    std::for_each(
        it,
        end,
        [&]( wild_cube & wc )
        {
            wc.right[1][2] =
            { c.left[1][1] };
            wc.back[1][2] =
            { c.back[1][1] };
        } );
    ++it;
    std::for_each(
        it,
        end,
        [&]( wild_cube & wc )
        {
            wc.front[0][1] =
            wc.front[2][1] =
            {
                c.front[1][1],
                c.back[1][1]
            };
        } );
    ++it;
    std::for_each(
        it,
        end,
        [&]( wild_cube & wc )
        {
            wc.back[0][1] =
            wc.back[2][1] =
            {
                c.front[1][1],
                c.back[1][1]
            };
        } );
    ++it;
    std::for_each(
        it,
        end,
        [&]( wild_cube & wc )
        {
            wc.up[0][1] =
            wc.up[2][1] =
            { c.up[1][1] };
        } );
    ++it;
    std::for_each(
        it,
        end,
        [&]( wild_cube & wc )
        {
            wc.down[0][1] =
            wc.down[2][1] =
            { c.down[1][1] };
        } );
    ++it;
    std::for_each(
        it,
        end,
        [&]( wild_cube & wc )
        {
            wc.front[0][1] =
            wc.front[2][1] =
            { c.front[1][1] };
        } );
    ++it;
    std::for_each(
        it,
        end,
        [&]( wild_cube & wc )
        {
            wc.back[0][1] =
            wc.back[2][1] =
            { c.back[1][1] };
        } );
    assert( it == end );
    auto all_action =
        []( const wild_cube &, auto it ) { std::copy( all_moves( ).begin( ), all_moves( ).end( ), it ); };
    auto f1 =
        []( wild_cube c, move m )
        {
            c.make_move( m );
            return c;
        };
    std::list< wild_cube > finished_middle_steps = middle_steps;
    for ( wild_cube & c : middle_steps ) { c.make_moves( moves.begin( ), moves.end( ) ); }
    while ( ! middle_steps.empty( ) )
    {
        std::list< move > forward, backward;
        AI::biderectional_breadth_first_search< move, move >(
            middle_steps.front( ),
            finished_middle_steps.front( ),
            all_action,
            f1,
            std::back_insert_iterator< decltype( forward ) >( forward ),
            all_action,
            f1,
            std::back_insert_iterator< decltype( backward ) >( backward ) );
        c.make_moves( forward.begin( ), forward.end( ) );
        c.undo_moves( backward.rbegin( ), backward.rend( ) );
        std::cout << c << std::endl;
        middle_steps.pop_front( );
        finished_middle_steps.pop_front( );
        for ( wild_cube & wc : middle_steps )
        {
            wc.make_moves( forward.begin( ), forward.end( ) );
            wc.undo_moves( backward.rbegin( ), backward.rend( ) );
        }
    }
    return 0;
}
