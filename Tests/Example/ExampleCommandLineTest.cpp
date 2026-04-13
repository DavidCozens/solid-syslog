#include "CppUTest/TestHarness.h"
#include "ExampleCommandLine.h"

#include <getopt.h>

// clang-format off
TEST_GROUP(ExampleCommandLine)
{
    struct ExampleOptions options = {};

    void setup() override
    {
        optind = 1;
    }

    int Parse(int argc, char* argv[])
    {
        return ExampleCommandLine_Parse(argc, argv, &options);
    }
};

// clang-format on

TEST(ExampleCommandLine, DefaultMaxFiles)
{
    char  arg0[] = "test";
    char* argv[] = {arg0, nullptr};
    Parse(1, argv);
    LONGS_EQUAL(10, options.maxFiles);
}

TEST(ExampleCommandLine, DefaultMaxFileSize)
{
    char  arg0[] = "test";
    char* argv[] = {arg0, nullptr};
    Parse(1, argv);
    LONGS_EQUAL(65536, options.maxFileSize);
}

TEST(ExampleCommandLine, DefaultDiscardPolicy)
{
    char  arg0[] = "test";
    char* argv[] = {arg0, nullptr};
    Parse(1, argv);
    STRCMP_EQUAL("oldest", options.discardPolicy);
}

TEST(ExampleCommandLine, DefaultNoSd)
{
    char  arg0[] = "test";
    char* argv[] = {arg0, nullptr};
    Parse(1, argv);
    CHECK_FALSE(options.noSd);
}

TEST(ExampleCommandLine, MaxFilesFlag)
{
    char  arg0[] = "test";
    char  arg1[] = "--max-files";
    char  arg2[] = "5";
    char* argv[] = {arg0, arg1, arg2, nullptr};
    LONGS_EQUAL(0, Parse(3, argv));
    LONGS_EQUAL(5, options.maxFiles);
}

TEST(ExampleCommandLine, MaxFileSizeFlag)
{
    char  arg0[] = "test";
    char  arg1[] = "--max-file-size";
    char  arg2[] = "1024";
    char* argv[] = {arg0, arg1, arg2, nullptr};
    LONGS_EQUAL(0, Parse(3, argv));
    LONGS_EQUAL(1024, options.maxFileSize);
}

TEST(ExampleCommandLine, DiscardPolicyOldest)
{
    char  arg0[] = "test";
    char  arg1[] = "--discard-policy";
    char  arg2[] = "oldest";
    char* argv[] = {arg0, arg1, arg2, nullptr};
    LONGS_EQUAL(0, Parse(3, argv));
    STRCMP_EQUAL("oldest", options.discardPolicy);
}

TEST(ExampleCommandLine, DiscardPolicyNewest)
{
    char  arg0[] = "test";
    char  arg1[] = "--discard-policy";
    char  arg2[] = "newest";
    char* argv[] = {arg0, arg1, arg2, nullptr};
    LONGS_EQUAL(0, Parse(3, argv));
    STRCMP_EQUAL("newest", options.discardPolicy);
}

TEST(ExampleCommandLine, DiscardPolicyHalt)
{
    char  arg0[] = "test";
    char  arg1[] = "--discard-policy";
    char  arg2[] = "halt";
    char* argv[] = {arg0, arg1, arg2, nullptr};
    LONGS_EQUAL(0, Parse(3, argv));
    STRCMP_EQUAL("halt", options.discardPolicy);
}

TEST(ExampleCommandLine, InvalidDiscardPolicyReturnsOne)
{
    char  arg0[] = "test";
    char  arg1[] = "--discard-policy";
    char  arg2[] = "invalid";
    char* argv[] = {arg0, arg1, arg2, nullptr};
    LONGS_EQUAL(1, Parse(3, argv));
}

TEST(ExampleCommandLine, NoSdFlag)
{
    char  arg0[] = "test";
    char  arg1[] = "--no-sd";
    char* argv[] = {arg0, arg1, nullptr};
    LONGS_EQUAL(0, Parse(2, argv));
    CHECK_TRUE(options.noSd);
}
