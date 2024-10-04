#pragma once

#if 0


#ifndef WE_DEBUG
#define WE_DEBUG(...) ((void)0)
#endif

#ifndef WE_ASSERT
#define WE_ASSERT(...) ((void)0)
#endif


#else


#ifndef WE_DEBUG
#define WE_DEBUG(...) ::we::logger().debug(__VA_ARGS__)
#endif

#ifndef WE_ASSERT
#define WE_ASSERT(expression, message)                                                   \
    (void)((!!(expression))                                                              \
           || (::we::logger().debug("assert {0} failed, {1}", #expression, message),     \
               std::terminate(),                                                         \
               0))
#endif


#endif
