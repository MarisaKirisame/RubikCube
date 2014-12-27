#include <thread>
#include <iostream>
#include <memory>
#include <array>
#include <algorithm>
#include <utility>
#include <iterator>
#include <atomic>
#include "../Artificial_Intelligence_A_Modern_Approach/search.hpp"
enum class color { red, green, blue, yellow, orange, white };
constexpr size_t cube_length = 3;/*Change rotate and position
                                  *if you want to change this*/
static const std::array< std::array< double, cube_length >, cube_length > front_misplaced_penalty =
{{
    {{ 10000, 10000, 10000 }},
    {{ 10000, 10000, 10000 }},
    {{ 10000, 10000, 10000 }}
}};
static const std::array< std::array< double, cube_length >, cube_length > back_misplaced_penalty =
{{
    {{ 0, 0, 0 }},
    {{ 0, 0, 0 }},
    {{ 0, 0, 0 }}
}};
static const std::array< std::array< double, cube_length >, cube_length > down_misplaced_penalty =
{{
    {{ 100, 100, 100 }},
    {{   1,   1,   1 }},
    {{ .01, .01, .01 }}
}};
static const std::array< std::array< double, cube_length >, cube_length > left_misplaced_penalty =
{{
    {{ .01, 1, 100 }},
    {{ .01, 1, 100 }},
    {{ .01, 1, 100 }}
}};
static const std::array< std::array< double, cube_length >, cube_length > right_misplaced_penalty =
{{
    {{ 100, 1, .01 }},
    {{ 100, 1, .01 }},
    {{ 100, 1, .01 }}
}};
static const std::array< std::array< double, cube_length >, cube_length > up_misplaced_penalty =
{{
    {{ .01, .01, .01 }},
    {{   1,   1,   1 }},
    {{ 100, 100, 100 }}
}};
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
    double misplaced_square_penalty(
        color original_color, const std::array< std::array< double, cube_length >, cube_length > & p ) const
    {
        auto ran = make_iterator_iterator( data.begin( ), data.end( ) );
        auto ran2 = make_iterator_iterator( p.begin( ), p.end( ) );
        auto it = ran.first;
        double acc = 0;
        for ( double pen : make_range_container_proxy( ran2.first, ran2.second ) )
        {
            acc += (*it == original_color ? 0.0 : pen);
            ++it;
        }
        return acc;
    }
    bool operator < ( const faces & cmp ) const { return data < cmp.data; }
    bool operator == ( const faces & cmp ) const { return data == cmp.data; }
    bool is_fix( color original_color ) const
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
    enum which_face { left, right, front, back, up, down, middle, equator, standing } wf;
    enum degree { clockwise, counter_clockwise, flip } dg;
    move( which_face wf, degree dg ) : wf( wf ), dg( dg ) { }
};

template< typename OS >
OS & operator << ( OS & os, const move & m )
{
    return os << (
        m.wf == move::left ? 'L' :
        m.wf == move::right ? 'R' :
        m.wf == move::front ? 'F' :
        m.wf == move::back ? 'B' :
        m.wf == move::up ? 'U' :
        m.wf == move::down ? 'D' :
        m.wf == move::middle ? 'M' :
        m.wf == move::equator ? 'E' : 'S' ) << (
            m.dg == move::clockwise ? ' ' :
            m.dg == move::counter_clockwise ? '\'' : '2' );
}

struct cube
{
    faces front, back, left, right, up, down;
    void make_move( const move & m )
    {
        size_t repeat = m.dg == m.clockwise ? 1 : m.dg == m.flip ? 2 : 3;
        while ( repeat != 0 )
        {
            --repeat;
            switch ( m.wf )
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
    bool is_fix( ) const
    {
        return
            front.is_fix( color::blue ) && back.is_fix( color::green ) && left.is_fix( color::orange ) &&
            right.is_fix( color::red ) && up.is_fix( color::white ) && down.is_fix( color::yellow );
    }
    double misplaced_square_penalty( ) const
    {
        return
                front.misplaced_square_penalty( color::blue, front_misplaced_penalty ) +
                back.misplaced_square_penalty( color::green, back_misplaced_penalty ) +
                left.misplaced_square_penalty( color::orange, left_misplaced_penalty ) +
                right.misplaced_square_penalty( color::red, right_misplaced_penalty ) +
                up.misplaced_square_penalty( color::white, up_misplaced_penalty ) +
                down.misplaced_square_penalty( color::yellow, down_misplaced_penalty );
    }
    auto tie( ) const { return std::tie( front, back, left, right, up, down ); }
    bool operator < ( const cube & cmp ) const { return tie( ) < cmp.tie( ); }
    bool operator == ( const cube & cmp ) const { return tie( ) == cmp.tie( ); }
    cube( ) :
        front( color::blue ), back( color::green ), left( color::orange ),
        right( color::red ), up( color::white ), down( color::yellow ) { }
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
                [&inner]( move::which_face wf )
                {
                    inner.push_back( move( wf, move::clockwise ) );
                    inner.push_back( move( wf, move::counter_clockwise ) );
                    inner.push_back( move( wf, move::flip ) );
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

template< size_t N >
const std::vector< std::vector< move > > & generate( )
{
    static const std::vector< std::vector< move > > ret(
            []( )
            {
                const auto & smaller = generate< N - 1 >( );
                std::vector< std::vector< move > > inner;
                for ( const std::vector< move > & m : smaller )
                {
                    for ( const move & mv : all_moves( ) )
                    {
                        auto vm = m;
                        vm.push_back( mv );
                        inner.push_back( vm );
                    }
                    inner.push_back( m );
                }
                return inner;
            }( ) );
    return ret;
}

template< >
const std::vector< std::vector< move > > & generate< 0 >( )
{
    static const std::vector< std::vector< move > > ret = { { } };
    return ret;
}

int main( )
{
    cube c;
    c.make_move( move( move::up, move::clockwise ) );
    c.make_move( move( move::right, move::flip ) );
    c.make_move( move( move::front, move::clockwise ) );
    c.make_move( move( move::back, move::clockwise ) );
    c.make_move( move( move::right, move::clockwise ) );
    c.make_move( move( move::back, move::flip ) );
    c.make_move( move( move::right, move::clockwise ) );
    c.make_move( move( move::up, move::flip ) );
    c.make_move( move( move::left, move::clockwise ) );
    c.make_move( move( move::back, move::flip ) );
    c.make_move( move( move::right, move::clockwise ) );
    c.make_move( move( move::up, move::counter_clockwise ) );
    c.make_move( move( move::down, move::counter_clockwise ) );
    c.make_move( move( move::right, move::flip ) );
    c.make_move( move( move::front, move::clockwise ) );
    c.make_move( move( move::right, move::counter_clockwise ) );
    c.make_move( move( move::left, move::clockwise ) );
    c.make_move( move( move::back, move::flip ) );
    c.make_move( move( move::up, move::flip ) );
    c.make_move( move( move::front, move::flip ) );
    double best = std::numeric_limits< double >::max( );
    std::vector< std::vector< move > > ret;
    std::atomic_bool b;
    std::thread t(
        [&]( ){ while ( true )
        {
            std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
            b = true;
        } } );
    std::cout << c.misplaced_square_penalty( ) << std::endl;
    A_star< std::vector< move > >(
        c,
        static_cast< size_t >( 0 ),
        []( const cube &, auto it )
        {
            for ( const std::vector< move > & p : generate< 4 >( ) )
            {
                *it = std::make_pair( p, 0.1 );
                ++it;
            }
        },
        [&]( cube c, const std::vector< move > & m )
        {
            for ( move mm : m ) { c.make_move( mm ); }
            if ( c.misplaced_square_penalty( ) < best )
            {
                best = c.misplaced_square_penalty( );
                std::cout << "new champion found" << std::endl
                          << c << "score is" << best << std::endl;
                std::cin.get( );
            }
            if ( b )
            {
                b = ! b;
                std::cout << c << c.misplaced_square_penalty( ) << std::endl;
            }
            return c;
        },
        []( const cube & c ) { return c.misplaced_square_penalty( ); },
        []( const cube & c ) { return c.is_fix( ); },
        []( auto ){ },
        std::back_insert_iterator< decltype( ret ) >( ret ) );
    auto r = make_iterator_iterator( ret.begin( ), ret.end( ) );
    for ( const move & m : make_range_container_proxy( r.first, r.second ) )
    {
        std::cout << m << ' ';
        c.make_move( m );
    }
    std::cout << std::endl;
    t.detach( );
    std::cout << c << std::endl;
    assert( c.is_fix( ) );
    return 0;
}
