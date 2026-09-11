#include <unity.h>
#include <PlatformPocketCore.h>

void setUp()
{
}

void tearDown()
{
}

void test_signal_strong_range()
{
    TEST_ASSERT_EQUAL_STRING("STRONG", getSignalLabel(-40));
    TEST_ASSERT_EQUAL_STRING("STRONG", getSignalLabel(-50));
}

void test_signal_good_range()
{
    TEST_ASSERT_EQUAL_STRING("GOOD", getSignalLabel(-51));
    TEST_ASSERT_EQUAL_STRING("GOOD", getSignalLabel(-65));
}

void test_signal_fair_range()
{
    TEST_ASSERT_EQUAL_STRING("FAIR", getSignalLabel(-66));
    TEST_ASSERT_EQUAL_STRING("FAIR", getSignalLabel(-75));
}

void test_signal_weak_range()
{
    TEST_ASSERT_EQUAL_STRING("WEAK", getSignalLabel(-76));
    TEST_ASSERT_EQUAL_STRING("WEAK", getSignalLabel(-100));
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_signal_strong_range);
    RUN_TEST(test_signal_good_range);
    RUN_TEST(test_signal_fair_range);
    RUN_TEST(test_signal_weak_range);

    return UNITY_END();
}
