#pragma once

#include <cstdlib>
#include <iostream>

// some macro for testing
#define guarantee(cond, msg)                                                                                           \
    if (!(cond))                                                                                                       \
    {                                                                                                                  \
        std::cerr << __FILE__ ":" << __LINE__ << ": condition (" #cond ") failed: " msg << std::endl;                  \
        abort();                                                                                                       \
    }

#define guarantee_eq(lv, rv) guarantee(lv == rv, #lv " and " #rv " are not equal!")

#define guarantee_ne(lv, rv) guarantee(lv != rv, #lv " and " #rv " are equal!")

#define guarantee_null(val) guarantee(val == NULL, #val " is not null!")

#define guarantee_not_null(val) guarantee(val != NULL, #val " is null!")

// other macro
#define UNIMPEMENTED(method)                                                                                           \
    std::cerr << "Unimplemented method: " method << std::endl;                                                         \
    abort();