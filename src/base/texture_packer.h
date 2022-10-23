//
// Created by ChenXin on 2022/9/22.
//

#pragma once

#include <core/stl.h>
#include <base/pixel.h>
#include <core/logger.h>

namespace gl_render {

    class TexturePacker {

    public:
        struct Quad {
            uint32_t index{};
            uint32_t x{};
            uint32_t y{};
            uint32_t size{};

            constexpr Quad() noexcept = default;
            constexpr Quad(uint32_t index, uint32_t x, uint32_t y, uint32_t size) noexcept : index{index}, x{x}, y{y}, size{size} {}
            void print() const noexcept {
                GL_RENDER_INFO("Quad: index = {}, x = {}, y = {}, size = {}", index, x, y, size);
            }
        };

        struct ImageBlock {
            uint32_t index;
            uint2 offset;
            uint2 size;

            constexpr ImageBlock() noexcept : index{}, offset{}, size{} {}
            constexpr ImageBlock(uint32_t index, uint2 offset, uint2 size) noexcept : index{index}, offset{offset}, size{size} {}
            void print() const noexcept {
                GL_RENDER_INFO("ImageBlock: index: {}, offset: ({}, {}), size: ({}, {})",
                               index, offset.x, offset.y, size.x, size.y);
            }
        };

    private:
        size_t _max_size{};
        size_t _min_size{};
        size_t _max_level_count{};
        vector<queue<Quad>> _available_quads;
        vector<vector<uchar4>> _image_buffers;
        unordered_map<path, ImageBlock> _loaded_images;

        Quad _decompose_quad(Quad quad, size_t target_size, size_t level) noexcept;
        Quad _fit_image(size_t w, size_t h) noexcept;
        void _fill(TexturePacker::ImageBlock b, const uchar4 *data) noexcept;

    public:
        explicit TexturePacker(size_t max_size = 4096ul, size_t min_size = 16ul);
        ImageBlock load(const path &image_path);
        [[nodiscard]] size_t count() const noexcept;
        [[nodiscard]] size_t max_size() const noexcept;
        [[nodiscard]] const vector<uchar4> &image_buffer(size_t index) const noexcept;
        [[nodiscard]] uint32_t create_opengl_texture_array() const noexcept;

    };

}