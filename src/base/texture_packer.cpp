//
// Created by Mike Smith on 2019/9/18.
//

#include <base/texture_packer.h>

#include <stb/stb_image.h>
#include <glad/glad.h>
#include <core/util.h>

namespace gl_render {

    TexturePacker::Quad TexturePacker::_decompose_quad(
            TexturePacker::Quad quad, size_t target_size, size_t level) noexcept {
        if (quad.size == target_size) {
            return quad;
        }
        level++;
        _available_quads[level].emplace(quad.index, quad.x + quad.size / 2, quad.y, quad.size / 2);
        _available_quads[level].emplace(quad.index, quad.x, quad.y + quad.size / 2, quad.size / 2);
        _available_quads[level].emplace(quad.index, quad.x + quad.size / 2, quad.y + quad.size / 2, quad.size / 2);
        return _decompose_quad({quad.index, quad.x, quad.y, quad.size / 2}, target_size, level);
    }

    TexturePacker::Quad TexturePacker::_fit_image(size_t w, size_t h) noexcept {
        auto size = std::max({util::next_power_of_two(w), util::next_power_of_two(h), _min_size});
        auto level = util::log2(_max_size / size);

        for (auto i = static_cast<int64_t>(level); i >= 0; i--) {
            auto &&quads = _available_quads[i];
            if (!quads.empty()) {
                auto quad = quads.front();
                quads.pop();
                return _decompose_quad(quad, size, i);
            }
        }
        auto quad =  _decompose_quad({static_cast<uint32_t>(_image_buffers.size()), 0, 0, static_cast<uint32_t>(_max_size)}, size, 0);
        _image_buffers.emplace_back(_max_size * _max_size, uchar4{0, 255, 0, 255});
        return quad;
    }

    void TexturePacker::_fill(TexturePacker::ImageBlock b, const uchar4 *data) noexcept {
        auto buffer = _image_buffers[b.index].data();
        for (auto row = 0ul; row < b.size.y; row++) {
            auto buffer_offset = (row + b.offset.y) * _max_size + b.offset.x;
            auto data_offset = row * b.size.x;
            memmove(buffer + buffer_offset, data + data_offset, b.size.x * sizeof(uchar4));
        }
    }

    TexturePacker::TexturePacker(size_t max_size, size_t min_size)
            : _max_size{util::next_power_of_two(max_size)},
              _min_size{util::next_power_of_two(min_size)} {
        _max_level_count = util::log2(_max_size / _min_size) + 1;
        _available_quads.resize(_max_level_count);
    }

    TexturePacker::ImageBlock TexturePacker::load(const path &image_path) {
        if (auto iter = _loaded_images.find(image_path); iter != _loaded_images.end()) {
            GL_RENDER_INFO("Using cached image: {}", image_path.string());
            return iter->second;
        }

        auto w = 0;
        auto h = 0;
        auto d = 0;

        GL_RENDER_INFO("Loading image: {}", image_path.string());
        auto deleter = [](uchar4 *p) noexcept { stbi_image_free(p); };
        unique_ptr<uchar4, decltype(deleter)> image_date{
                reinterpret_cast<uchar4 *>(stbi_load(image_path.string().c_str(), &w, &h, &d, 4)), deleter};

        if (!image_date || w > _max_size || h > _max_size) {
            GL_RENDER_ERROR_WITH_LOCATION("Failed to load image: {}", image_path.string());
        }

        auto quad = _fit_image(w, h);
        ImageBlock block{quad.index, {quad.x, quad.y}, {w, h}};
        _fill(block, image_date.get());
        _loaded_images.emplace(image_path, block);

        quad.print();
        block.print();

        return block;
    }

    size_t TexturePacker::count() const noexcept {
        return _image_buffers.size();
    }

    const vector<uchar4> &TexturePacker::image_buffer(size_t index) const noexcept {
        return _image_buffers[index];
    }

    size_t TexturePacker::max_size() const noexcept {
        return _max_size;
    }

    uint32_t TexturePacker::create_opengl_texture_array() const noexcept {
        static int index = 0;
        // test loaded textures
        for (auto i = 0ul; i < _image_buffers.size(); i++) {
            stbi_write_png(serialize("outputs/texture_packed/", i + index, ".png").c_str(),
                           _max_size, _max_size, 4, _image_buffers[i].data(), 0);
        }
        index += _image_buffers.size();

        uint32_t texture_array = 0u;
        glGenTextures(1, &texture_array);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture_array);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, _max_size, _max_size, _image_buffers.size(), 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        for (auto i = 0ul; i < _image_buffers.size(); i++) {
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, _max_size, _max_size, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                            _image_buffers[i].data());
        }
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        GL_RENDER_INFO("Created texture array");
        return texture_array;
    }

}