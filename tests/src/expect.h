#include <core/logger.h>
#include <math/lmath.h>

/**
 * @brief Expects expected to be equal to actual.
 * 
 */
#define expect_should_be(expected, actual)                                                              \
    if(actual != expected) {                                                                            \
        LERROR("--> Expected %lld, but got: %lld. File: %s:%d", expected, actual, __FILE__, __LINE__);  \
        return false;                                                                                   \
    }                                                                                                  

/**
 * @brief Expects expected to NOT be equal to actual.
 * 
 */
#define expect_should_not_be(expected, actual)                                                                      \
    if(actual == expected) {                                                                                        \
        LERROR("--> Expected %d != %d, but are actually equal. File: %s:%d", expected, actual, __FILE__, __LINE__); \
        return false;                                                                                               \
    }                                                                                                  

/**
 * @brief Expects expected to be equal to be actual given a tolerance of 0.001f.
 * L_EPS _might_ be used in the future. 
 */
#define expect_float_to_be(expected, actual)                                                        \
    if(labsf(expected - actual)  > 0.001f) {                                                        \
        LERROR("--> Expected %f, but got: %f. File: %s:%d", expected, actual, __FILE__, __LINE__);  \
        return false;                                                                               \
    }                                                                                                  

/**
 * @brief Expects expected to be true.
 * 
 */
#define expect_to_be_true(actual)                                                      \
    if(actual != true) {                                                               \
        LERROR("--> Expected true, but got: false. File: %s:%d", __FILE__, __LINE__);  \
        return false;                                                                  \
    } 

/**
 * @brief Expects expected to be false.
 * 
 */
#define expect_to_be_false(actual)                                                     \
    if(actual != false) {                                                              \
        LERROR("--> Expected false, but got: true. File: %s:%d", __FILE__, __LINE__);  \
        return false;                                                                  \
    }                                                                                                  
