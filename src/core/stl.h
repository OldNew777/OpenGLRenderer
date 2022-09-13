//
// Created by Kasumi on 2022/9/13.
//

#ifndef OPENGLRENDERER_STL_H
#define OPENGLRENDERER_STL_H

#include <cstdlib>
#include <cmath>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <fmt/format.h>

#include <EASTL/bit.h>
#include <EASTL/span.h>
#include <EASTL/list.h>
#include <EASTL/slist.h>
#include <EASTL/deque.h>
#include <EASTL/queue.h>
#include <EASTL/memory.h>
#include <EASTL/vector.h>
#include <EASTL/variant.h>
#include <EASTL/optional.h>
#include <EASTL/bitvector.h>
#include <EASTL/fixed_map.h>
#include <EASTL/fixed_set.h>
#include <EASTL/unique_ptr.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/functional.h>
#include <EASTL/vector_map.h>
#include <EASTL/vector_set.h>
#include <EASTL/shared_array.h>
#include <EASTL/fixed_hash_map.h>
#include <EASTL/fixed_hash_set.h>
#include <EASTL/vector_multimap.h>
#include <EASTL/vector_multiset.h>
#include <EASTL/bonus/lru_cache.h>

namespace gl_render {

    using string = std::basic_string<char, std::char_traits<char>>;
    using std::string_view;

    using eastl::const_pointer_cast;
    using eastl::dynamic_pointer_cast;
    using eastl::enable_shared_from_this;
    using eastl::function;
    using eastl::make_shared;
    using eastl::make_unique;
    using eastl::reinterpret_pointer_cast;
    using eastl::shared_ptr;
    using eastl::span;
    using eastl::static_pointer_cast;
    using eastl::unique_ptr;
    using eastl::weak_ptr;

}

#endif //OPENGLRENDERER_STL_H
