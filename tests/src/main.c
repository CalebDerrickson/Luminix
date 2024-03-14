/*
* A unit test should always result in a boolean - ie. did this test succeed (true) 
    or did it fail (false) - there can be no middle ground!

* Never write a unit test that causes an assertion or an exit - run all unit tests 
    if possible.

* Always write unit tests, never assume that you are too good!! If you have tried 
    just once joining a company with a big code base with no tests (and being unable 
    to know if what you are writing will work), then you will understand the importance.

* Write tests covering all features related to a concept, ie. all functions for an 
    allocator or all methods for a class.

* If you are testing stuff that's meant to be multi-threaded then your tests need to 
    also be multi-threaded!

* Always output the parameters for a failed test to the console - sometimes these console
    outputs can be sent over a network (e.g. on e-mail).

* Use test-driven development (write tests first then implement code) when you in advance 
    know what the code is meant to do. That what you cannot forget something.

* Test all possible branches of code, ie. insert parameters to functions to walk through 
    all if-statements and switch-cases. Only then can you for certain tell your customers that the code works.

* If you have multiple build configurations for your code (e.g. debug, release, etc.), then 
    you must also run the unit tests for all these build configurations!

* Run the unit tests on all intended platforms. In the case of Kohi this would be Windows, 
    Linux, and MacOSX. Might not be possible on something some devices, but as far as possible 
    this would be considered good practice.

* Never be lazy. If you slack on testing it will come back to haunt you later!!
*/

#include "test_manager.h"

#include "memory/linear_allocator_tests.h"
#include "containers/hashtable_tests.h"
#include "containers/freelist_tests.h"

#include <core/logger.h>

int main()
{
    // Always initialize the test manager first.
    test_manager_init();

    // TODO: Add test registration here
    linear_allocator_register_tests();
    hashtable_register_tests();
    freelist_register_tests();

    LDEBUG("Starting tests...");
    // Execute tests
    test_manager_run_tests();

    return 0;

}