#include <unity.h>
#include <PlatformPocketCore.h>

/**
 * @brief Document setUp.
 */
void setUp()
{
}

/**
 * @brief Document tearDown.
 */
void tearDown()
{
}

/**
 * @brief Document test strong signal.
 */
void test_strong_signal()
{
    TEST_ASSERT_EQUAL_STRING(
        "STRONG",
        getSignalLabel(-40)
    );
}

/**
 * @brief Document main.
 */
int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_strong_signal);

    return UNITY_END();
}
