#include <unity.h>
#include <PlatformPocketCore.h>

void setUp()
{
}

void tearDown()
{
}

void test_strong_signal()
{
    TEST_ASSERT_EQUAL_STRING(
        "STRONG",
        getSignalLabel(-40)
    );
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_strong_signal);

    return UNITY_END();
}