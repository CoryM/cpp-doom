
#ifndef __TEST_D_EVENT_HPP__
#define __TEST_D_EVENT_HPP__

#include "catch2/catch.hpp"
#include "../d_event.hpp"

TEST_CASE("{d_event}", "[d_event][sideeffects]")
{
    auto ev1 = event_t({ evtype_t::ev_keydown, 1, 2, 3, 4, 5 });
    auto ev2 = event_t({ evtype_t::ev_keyup, 6, 7, 8, 9, 10 });

    SECTION("push onto event stack")
    {
        D_PostEvent(&ev1);
        D_PostEvent(&ev2);
    }

    SECTION("pop from event stack")
    {
        auto test1 = D_PopEvent();
        CHECK(ev1.type  == test1->type);
        CHECK(ev1.data1 == test1->data1);
        CHECK(ev1.data2 == test1->data2);
        CHECK(ev1.data3 == test1->data3);
        CHECK(ev1.data4 == test1->data4);
        CHECK(ev1.data5 == test1->data5);

        auto test2 = D_PopEvent();
        CHECK(ev2.type  == test2->type);
        CHECK(ev2.data1 == test2->data1);
        CHECK(ev2.data2 == test2->data2);
        CHECK(ev2.data3 == test2->data3);
        CHECK(ev2.data4 == test2->data4);
        CHECK(ev2.data5 == test2->data5);
    }

    SECTION("pop from empty stack")
    {
        auto test1 = D_PopEvent();
        CHECK(test1 == nullptr);
    }
}

#endif //__TEST_D_EVENT_HPP__