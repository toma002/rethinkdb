// Copyright 2010-2014 RethinkDB, all rights reserved.
#include "buffer_cache/alt/alt.hpp"
#include "unittest/gtest.hpp"
#include "unittest/memory_serializer.hpp"
#include "unittest/unittest_utils.hpp"

namespace unittest { namespace alt_test {

class wrapper_t : public memory_serializer_t, public cache_t {
public:
    wrapper_t() : cache_t(memory_serializer_t::ser.get(),
                          alt_cache_config_t(),
                          &get_global_perfmon_collection()) { }
};

void run_CreateDestroy() {
    wrapper_t c;
}

TEST(AltTest, CreateDestroy) {
    run_in_thread_pool(run_CreateDestroy, 4);
}

} }  // namespace unittest::alt_test
