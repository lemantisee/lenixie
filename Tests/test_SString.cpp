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

    SString<4> str4;
    str4.append("abcde", 4);
    REQUIRE(str4 == "abcd");
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

TEST_CASE("SString: remove symbol")
{
    SString<5> str1 = "abcde";
    str1.removeSymbol('c');
    REQUIRE(str1 == "abde");
    REQUIRE(std::strcmp(str1.c_str(), "abde") == 0);
    REQUIRE(str1.size() == 4);

    str1.removeSymbol('a');
    REQUIRE(str1 == "bde");
    REQUIRE(std::strcmp(str1.c_str(), "bde") == 0);
    REQUIRE(str1.size() == 3);

    str1.removeSymbol('e');
    REQUIRE(str1 == "bd");
    REQUIRE(std::strcmp(str1.c_str(), "bd") == 0);
    REQUIRE(str1.size() == 2);

    SString<6> str2 = "abadea";
    str2.removeSymbol('a');
    REQUIRE(str2 == "bde");
    REQUIRE(std::strcmp(str2.c_str(), "bde") == 0);
    REQUIRE(str2.size() == 3);

    str2 = "aabdea";
    str2.removeSymbol('a');
    REQUIRE(str2 == "bde");
    REQUIRE(std::strcmp(str2.c_str(), "bde") == 0);
    REQUIRE(str2.size() == 3);

    str2 = "bdeaa";
    str2.removeSymbol('a');
    REQUIRE(str2 == "bde");
    REQUIRE(std::strcmp(str2.c_str(), "bde") == 0);
    REQUIRE(str2.size() == 3);
}