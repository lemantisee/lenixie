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

    str += "ab";
    REQUIRE(str == "HelloWorldWorldcab");

    str += 'c';
    REQUIRE(str == "HelloWorldWorldcabc");

    SString<128> str2 = " abc";
    SString<128> newstr = str + str2;
    REQUIRE(newstr == "HelloWorldWorldcabc abc");

    SString<4> str3 = "abcd";
    REQUIRE(str3 == "abcd");
}

TEST_CASE("SString: append number")
{
    SString<128> str = "Hello";
    REQUIRE(str.appendNumber(12) == "Hello12");
    REQUIRE(str.appendNumber(34, "%i") == "Hello1234");
    REQUIRE(str.appendNumber(5, "%02i") == "Hello123405");
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

TEST_CASE("SString to int")
{
    SString<4> str1 = "2024";
    REQUIRE(str1.toInt() == 2024);

    SString<4> str2 = "024";
    REQUIRE(str2.toInt() == 24);
}