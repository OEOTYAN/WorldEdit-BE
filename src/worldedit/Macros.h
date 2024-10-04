#pragma once

#ifndef WE_DEBUG
#define WE_DEBUG(...) ::we::logger().debug(__VA_ARGS__)
#endif

#ifndef WE_ASSERT
#define WE_ASSERT(expression, message)                                                   \
    (void)((!!(expression))                                                              \
           || (::we::logger().debug("assert {} failed, {}", #expression, message),       \
               std::terminate(),                                                         \
               0))
#endif
