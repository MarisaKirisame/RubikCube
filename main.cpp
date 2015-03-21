#include "cube.hpp"

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
