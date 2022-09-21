//
// Created by chenxin on 2022/9/21.
//

#pragma once

#include <core/stl.h>

namespace gl_render {

    namespace impl {

        template<typename ...Args>
        struct StreamFeederImpl {};

        template<typename FirstArg, typename ...OtherArgs>
        struct StreamFeederImpl<FirstArg, OtherArgs...> {
            template<typename Stream>
            static Stream &feed(Stream &s, FirstArg &&first, OtherArgs &&...other) {
                s << std::forward<FirstArg>(first);
                return StreamFeederImpl<OtherArgs...>::feed(s, std::forward<OtherArgs>(other)...);
            }
        };

        template<>
        struct StreamFeederImpl<> {
            template<typename Stream>
            static Stream &feed(Stream &s) {
                return s;
            }
        };

        template<typename Stream, typename ...Args>
        Stream &feed_stream(Stream &recv, Args &&...args) {
            return StreamFeederImpl<Args...>::feed(recv, std::forward<Args>(args)...);
        }

    }

    template<typename ...Args>
    std::string serialize(Args &&...args) {
        std::stringstream ss;
        return impl::feed_stream(ss, std::forward<Args>(args)...).str();
    }

}