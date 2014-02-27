// Copyright 2010-2014 RethinkDB, all rights reserved.
#include "buffer_cache/alt/alt.hpp"
#include "serializer/config.hpp"
#include "unittest/gtest.hpp"
#include "unittest/mock_file.hpp"

namespace unittest {

class wrapper_t {
public:
    wrapper_t()
        : opener() {
        standard_serializer_t::create(&opener,
                                      standard_serializer_t::static_config_t());
        ser = make_scoped<standard_serializer_t>(log_serializer_t::dynamic_config_t(),
                                                 &opener,
                                                 &get_global_perfmon_collection());
        cache = make_scoped<cache_t>(ser.get(), alt_cache_config_t(),
                                     &get_global_perfmon_collection());
    };

    mock_file_opener_t opener;
    scoped_ptr_t<standard_serializer_t> ser;
    scoped_ptr_t<cache_t> cache;
};

}  // namespace unittest
