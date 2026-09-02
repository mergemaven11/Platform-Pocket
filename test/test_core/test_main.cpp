#include <unity.h>
#include <PlatformPocketCore.h>

/**
 * @brief Handle setUp.
 */
void setUp()
{
}

/**
 * @brief Handle tearDown.
 */
void tearDown()
{
}

/**
 * @brief Handle test strong signal.
 */
void test_strong_signal()
{
    /**
     * @brief Handle TEST ASSERT EQUAL STRING.
     */
    TEST_ASSERT_EQUAL_STRING(
        "STRONG",
        getSignalLabel(-40)
    );
}

/**
 * @brief Handle main.
 */
int main()
{
    /**
     * @brief Handle UNITY BEGIN.
     */
    UNITY_BEGIN();

    /**
     * @brief Handle RUN TEST.
     */
    RUN_TEST(test_strong_signal);

    return UNITY_END();
}
