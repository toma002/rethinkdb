// Copyright 2010-2014 RethinkDB, all rights reserved.
#ifndef UNITTEST_MEMORY_SERIALIZER_HPP_
#define UNITTEST_MEMORY_SERIALIZER_HPP_

#include "serializer/config.hpp"
#include "unittest/mock_file.hpp"

namespace unittest {

class memory_serializer_t {
public:
    mock_file_opener_t opener;
    scoped_ptr_t<standard_serializer_t> ser;

    standard_serializer_t *get() {
        return ser.get();
    }

    memory_serializer_t()
        : opener() {
        standard_serializer_t::create(&opener,
                                      standard_serializer_t::static_config_t());
        ser = make_scoped<standard_serializer_t>(log_serializer_t::dynamic_config_t(),
                                                 &opener,
                                                 &get_global_perfmon_collection());
    }

    DISABLE_COPYING(memory_serializer_t);
};


}  // namespace unittest

#endif  // UNITTEST_MEMORY_SERIALIZER_HPP_
