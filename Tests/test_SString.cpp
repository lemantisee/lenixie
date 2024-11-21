#include <catch2/catch_test_macros.hpp>

#include "SString.h"

TEST_CASE("SString: capacity")
{
    SString<128> str;

    REQUIRE(str.capacity() == 128);
}

TEST_CASE("SString: size")
{
    SString<128> str;

    REQUIRE(str.size() == 0);
    REQUIRE(str.empty());

    str = "Hello World";

    REQUIRE(str.size() == 11);
    REQUIRE_FALSE(str.empty());
}

TEST_CASE("SString: resize")
{
    SString<128> str = "Hello World";

    str.resize(10);
    REQUIRE(str == "Hello Worl");
    REQUIRE(str.size() == 10);

    str.resize(12);
    REQUIRE(str == "Hello Worl");
    REQUIRE(str.size() == 12);
}

TEST_CASE("SString: contains")
{
    SString<128> str = "Hello World World";

    REQUIRE(str.contains("World"));
    REQUIRE(*str.find("World") == 6);
    REQUIRE(*str.find("World", 7) == 12);
    REQUIRE(str.find("asd") == std::nullopt);
}

TEST_CASE("SString: append")
{
    SString<128> str = "Hello";

    REQUIRE(str.append("World") == "HelloWorld");
    REQUIRE(str.append("World!s", 5) == "HelloWorldWorld");
    REQUIRE(str.append('c') == "HelloWorldWorldc");
    REQUIRE(str.appendNumber(12) == "HelloWorldWorldc12");

    str += "ab";
    REQUIRE(str == "HelloWorldWorldc12ab");

    str += 'c';
    REQUIRE(str == "HelloWorldWorldc12abc");

    SString<128> str2 = " abc";
    SString<128> newstr = str + str2;
    REQUIRE(newstr == "HelloWorldWorldc12abc abc");
}

TEST_CASE("SString: operator[]")
{
    SString<128> str = "Hello";

    REQUIRE(str[0] == 'H');
    REQUIRE(str[1] == 'e');
    REQUIRE(str[2] == 'l');
    REQUIRE(str[3] == 'l');
    REQUIRE(str[4] == 'o');
    REQUIRE(str[5] == 0);
}

TEST_CASE("SString: back/pop")
{
    SString<128> str = "Hello";

    REQUIRE(str.back() == 'o');
    str.pop();
    REQUIRE(str.back() == 'l');
    REQUIRE(str.size() == 4);
}