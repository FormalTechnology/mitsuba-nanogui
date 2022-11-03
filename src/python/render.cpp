#ifdef NANOGUI_PYTHON

#include <nanobind/tensor.h>

#include "python.h"

static VariableType dtype_to_enoki(nb::dlpack::dtype dtype) {
    switch ((nb::dlpack::dtype_code) dtype.code) {
        case nb::dlpack::dtype_code::Int:
            switch (dtype.bits) {
                case 8: return VariableType::Int8;
                case 16: return VariableType::Int16;
                case 32: return VariableType::Int32;
                case 64: return VariableType::Int64;
                default: break;
            }
            break;

        case nb::dlpack::dtype_code::UInt:
            switch (dtype.bits) {
                case 8: return VariableType::UInt8;
                case 16: return VariableType::UInt16;
                case 32: return VariableType::UInt32;
                case 64: return VariableType::UInt64;
                default: break;
            }
            break;

        case nb::dlpack::dtype_code::Float:
            switch (dtype.bits) {
                case 16: return VariableType::Float16;
                case 32: return VariableType::Float32;
                case 64: return VariableType::Float64;
                default: break;
            }
            break;

        default:
            break;
    }

    return VariableType::Invalid;
}

static void
shader_set_buffer(Shader &shader, const std::string &name,
                  nb::tensor<nb::device::cpu, nb::c_contig> tensor) {
    if (tensor.ndim() > 3)
        throw nb::type_error("Shader::set_buffer(): tensor rank must be < 3!");

    VariableType dtype = dtype_to_enoki(tensor.dtype());

    if (dtype == VariableType::Invalid)
        throw nb::type_error("Shader::set_buffer(): unsupported array dtype!");

    size_t dim[3] {
        tensor.ndim() > 0 ? (size_t) tensor.shape(0) : 1,
        tensor.ndim() > 1 ? (size_t) tensor.shape(1) : 1,
        tensor.ndim() > 2 ? (size_t) tensor.shape(2) : 1
    };

    shader.set_buffer(name, dtype, tensor.ndim(), dim, tensor.data());
}

static nb::tensor<nb::numpy> texture_download(Texture &texture) {
    nb::dlpack::dtype dt;

    switch (texture.component_format()) {
        case Texture::ComponentFormat::Int8:    dt = nb::dtype<int8_t>(); break;
        case Texture::ComponentFormat::UInt8:   dt = nb::dtype<uint8_t>(); break;
        case Texture::ComponentFormat::Int16:   dt = nb::dtype<int16_t>(); break;
        case Texture::ComponentFormat::UInt16:  dt = nb::dtype<uint16_t>(); break;
        case Texture::ComponentFormat::Int32:   dt = nb::dtype<int32_t>(); break;
        case Texture::ComponentFormat::UInt32:  dt = nb::dtype<uint32_t>(); break;
        case Texture::ComponentFormat::Float16: dt = nb::dtype<float>(); dt.bits = 16; break;
        case Texture::ComponentFormat::Float32: dt = nb::dtype<float>(); break;
        default: throw std::runtime_error("Invalid component format");
    }

    // Dynamically allocate 'data'
    size_t shape[3] = { (size_t) texture.size().y(),
                        (size_t) texture.size().x(),
                        (size_t) texture.channels() };
    uint8_t *ptr = new uint8_t[shape[0] * shape[1] * shape[2] * dt.bits / 8];

    // Delete 'data' when the 'owner' capsule expires
    nb::capsule owner(ptr, [](void *p) noexcept {
       delete[] (uint8_t *) p;
    });

    texture.download(ptr);

    return nb::tensor<nb::numpy>(ptr, 3, shape, owner, nullptr, dt);
}

static void texture_upload(Texture &texture, nb::tensor<nb::device::cpu, nb::c_contig> tensor) {
    size_t n_channels = tensor.ndim() == 3 ? tensor.shape(2) : 1;
    VariableType dtype         = dtype_to_enoki(tensor.dtype()),
                 dtype_texture = (VariableType) texture.component_format();

    if (tensor.ndim() != 2 && tensor.ndim() != 3)
        throw std::runtime_error("Texture::upload(): expected a 2 or 3-dimensional array!");
    else if (tensor.shape(0) != (size_t) texture.size().y() ||
             tensor.shape(1) != (size_t) texture.size().x())
        throw std::runtime_error("Texture::upload(): array size does not match the texture!");
    else if (n_channels != texture.channels())
        throw std::runtime_error(
            std::string("Texture::upload(): number of color channels in array (") +
            std::to_string(n_channels) + ") does not match the texture (" +
            std::to_string(texture.channels()) + ")!");
    else if (dtype != dtype_texture)
        throw std::runtime_error(
            std::string("Texture::upload(): dtype of array (") +
            type_name(dtype) + ") does not match the texture (" +
            type_name(dtype_texture) + ")!");

    texture.upload((const uint8_t *) tensor.data());
}

static void texture_upload_sub_region(Texture &texture, nb::tensor<nb::device::cpu, nb::c_contig> tensor, const Vector2i &origin) {
    size_t n_channels = tensor.ndim() == 3 ? tensor.shape(2) : 1;
    VariableType dtype         = dtype_to_enoki(tensor.dtype()),
                 dtype_texture = (VariableType) texture.component_format();

    if (tensor.ndim() != 2 && tensor.ndim() != 3)
        throw std::runtime_error("Texture::upload_sub_region(): expected a 2 or 3-dimensional array!");
    else if (tensor.shape(0) + (size_t) origin.x() > (size_t) texture.size().y() ||
             tensor.shape(1) + (size_t) origin.y() > (size_t) texture.size().x())
        throw std::runtime_error("Texture::upload_sub_region(): bounds exceed the size of the texture!");
    else if (n_channels != texture.channels())
        throw std::runtime_error(
            std::string("Texture::upload_sub_region(): number of color channels in array (") +
            std::to_string(n_channels) + ") does not match the texture (" +
            std::to_string(texture.channels()) + ")!");
    else if (dtype != dtype_texture)
        throw std::runtime_error(
            std::string("Texture::upload_sub_region(): dtype of array (") +
            type_name(dtype) + ") does not match the texture (" +
            type_name(dtype_texture) + ")!");

    texture.upload_sub_region(
        (const uint8_t *) tensor.data(), origin,
        { (int32_t) tensor.shape(0), (int32_t) tensor.shape(1) });
}
#endif

void register_render(nb::module_ &m) {
    using PixelFormat       = Texture::PixelFormat;
    using ComponentFormat   = Texture::ComponentFormat;
    using InterpolationMode = Texture::InterpolationMode;
    using WrapMode          = Texture::WrapMode;
    using TextureFlags      = Texture::TextureFlags;
    using PrimitiveType     = Shader::PrimitiveType;
    using BlendMode         = Shader::BlendMode;
    using DepthTest         = RenderPass::DepthTest;
    using CullMode          = RenderPass::CullMode;

    auto texture = nb::class_<Texture, Object>(m, "Texture", D(Texture));

    nb::enum_<PixelFormat>(texture, "PixelFormat", D(Texture, PixelFormat))
        .value("R", PixelFormat::R, D(Texture, PixelFormat, R))
        .value("RA", PixelFormat::RA, D(Texture, PixelFormat, RA))
        .value("RGB", PixelFormat::RGB, D(Texture, PixelFormat, RGB))
        .value("RGBA", PixelFormat::RGBA, D(Texture, PixelFormat, RGBA))
        .value("BGR", PixelFormat::BGR, D(Texture, PixelFormat, BGR))
        .value("BGRA", PixelFormat::BGRA, D(Texture, PixelFormat, BGRA))
        .value("Depth", PixelFormat::Depth, D(Texture, PixelFormat, Depth))
        .value("DepthStencil", PixelFormat::DepthStencil, D(Texture, PixelFormat, DepthStencil));

    nb::enum_<ComponentFormat>(texture, "ComponentFormat", D(Texture, ComponentFormat))
        .value("UInt8", ComponentFormat::UInt8, D(Texture, ComponentFormat, UInt8))
        .value("Int8", ComponentFormat::Int8, D(Texture, ComponentFormat, Int8))
        .value("UInt16", ComponentFormat::UInt16, D(Texture, ComponentFormat, UInt16))
        .value("Int16", ComponentFormat::Int16, D(Texture, ComponentFormat, Int16))
        .value("UInt32", ComponentFormat::UInt32, D(Texture, ComponentFormat, UInt32))
        .value("Int32", ComponentFormat::Int32, D(Texture, ComponentFormat, Int32))
        .value("Float16", ComponentFormat::Float16, D(Texture, ComponentFormat, Float16))
        .value("Float32", ComponentFormat::Float32, D(Texture, ComponentFormat, Float32));

    nb::enum_<InterpolationMode>(texture, "InterpolationMode", D(Texture, InterpolationMode))
        .value("Nearest", InterpolationMode::Nearest, D(Texture, InterpolationMode, Nearest))
        .value("Bilinear", InterpolationMode::Bilinear, D(Texture, InterpolationMode, Bilinear))
        .value("Trilinear", InterpolationMode::Trilinear, D(Texture, InterpolationMode, Trilinear));

    nb::enum_<WrapMode>(texture, "WrapMode", D(Texture, WrapMode))
        .value("ClampToEdge", WrapMode::ClampToEdge, D(Texture, WrapMode, ClampToEdge))
        .value("Repeat", WrapMode::Repeat, D(Texture, WrapMode, Repeat))
        .value("MirrorRepeat", WrapMode::MirrorRepeat, D(Texture, WrapMode, MirrorRepeat));

    nb::enum_<TextureFlags>(texture, "TextureFlags", D(Texture, TextureFlags), nb::is_arithmetic())
        .value("ShaderRead", TextureFlags::ShaderRead, D(Texture, TextureFlags, ShaderRead))
        .value("RenderTarget", TextureFlags::RenderTarget, D(Texture, TextureFlags, RenderTarget));

    texture
        .def(nb::init<PixelFormat, ComponentFormat, const Vector2i &,
                      InterpolationMode, InterpolationMode, WrapMode, uint8_t, uint8_t, bool>(),
             D(Texture, Texture), "pixel_format"_a, "component_format"_a, "size"_a,
             "min_interpolation_mode"_a = InterpolationMode::Bilinear,
             "mag_interpolation_mode"_a = InterpolationMode::Bilinear,
             "wrap_mode"_a = WrapMode::ClampToEdge, "samples"_a = 1,
             "flags"_a = (uint8_t) TextureFlags::ShaderRead,
             "mipmap_manual"_a = false)
        .def(nb::init<const std::string &, InterpolationMode, InterpolationMode, WrapMode>(),
             D(Texture, Texture, 2), "filename"_a,
             "min_interpolation_mode"_a = InterpolationMode::Bilinear,
             "mag_interpolation_mode"_a = InterpolationMode::Bilinear,
             "wrap_mode"_a = WrapMode::ClampToEdge)
        .def(nb_method(Texture, pixel_format), D(Texture, pixel_format))
        .def(nb_method(Texture, component_format), D(Texture, component_format))
        .def(nb_method(Texture, min_interpolation_mode), D(Texture, min_interpolation_mode))
        .def(nb_method(Texture, mag_interpolation_mode), D(Texture, mag_interpolation_mode))
        .def(nb_method(Texture, wrap_mode), D(Texture, wrap_mode))
        .def(nb_method(Texture, samples), D(Texture, samples))
        .def(nb_method(Texture, flags), D(Texture, flags))
        .def(nb_method(Texture, size), D(Texture, size))
        .def(nb_method(Texture, bytes_per_pixel), D(Texture, bytes_per_pixel))
        .def(nb_method(Texture, channels), D(Texture, channels))
        .def("download", &texture_download, D(Texture, download))
        .def("upload", &texture_upload, D(Texture, upload))
        .def("upload_sub_region", &texture_upload_sub_region, D(Texture, upload, origin))
        .def(nb_method(Texture, generate_mipmap), D(Texture, generate_mipmap))
        .def(nb_method(Texture, resize), D(Texture, resize))
#if defined(NANOGUI_USE_OPENGL) || defined(NANOGUI_USE_GLES)
        .def(nb_method(Texture, texture_handle))
        .def(nb_method(Texture, renderbuffer_handle))
#elif defined(NANOGUI_USE_METAL)
        .def(nb_method(Texture, texture_handle))
        .def(nb_method(Texture, sampler_state_handle))
#endif
        ;

    auto shader = nb::class_<Shader, Object>(m, "Shader", D(Shader));

    nb::enum_<BlendMode>(shader, "BlendMode", D(Shader, BlendMode))
        .value("None", BlendMode::None, D(Shader, BlendMode, None))
        .value("AlphaBlend", BlendMode::AlphaBlend, D(Shader, BlendMode, AlphaBlend));

    shader
        .def(nb::init<RenderPass *, const std::string &,
                      const std::string &, const std::string &, Shader::BlendMode>(),
             D(Shader, Shader), "render_pass"_a, "name"_a, "vertex_shader"_a,
             "fragment_shader"_a, "blend_mode"_a = BlendMode::None)
        .def(nb_method(Shader, name), D(Shader, name))
        .def(nb_method(Shader, blend_mode), D(Shader, blend_mode))
        .def("set_buffer", &shader_set_buffer, D(Shader, set_buffer))
        .def(nb_method(Shader, set_texture), D(Shader, set_texture))
        .def(nb_method(Shader, begin), D(Shader, begin))
        .def(nb_method(Shader, end), D(Shader, end))
        .def("__enter__", &Shader::begin)
        .def("__exit__", [](Shader &s, nb::handle, nb::handle, nb::handle) { s.end(); },
             "type"_a.none(), "value"_a.none(), "traceback"_a.none())
        .def(nb_method(Shader, draw_array), D(Shader, draw_array),
             "primitive_type"_a, "offset"_a, "count"_a, "indexed"_a = false)
#if defined(NANOGUI_USE_OPENGL) || defined(NANOGUI_USE_GLES)
        .def(nb_method(Shader, shader_handle))
#elif defined(NANOGUI_USE_METAL)
        .def(nb_method(Shader, pipeline_state))
#endif
#if defined(NANOGUI_USE_OPENGL)
        .def(nb_method(Shader, vertex_array_handle))
#endif
        ;

    nb::enum_<PrimitiveType>(shader, "PrimitiveType", D(Shader, PrimitiveType))
        .value("Point", PrimitiveType::Point, D(Shader, PrimitiveType, Point))
        .value("Line", PrimitiveType::Line, D(Shader, PrimitiveType, Line))
        .value("LineStrip", PrimitiveType::LineStrip, D(Shader, PrimitiveType, LineStrip))
        .value("Triangle", PrimitiveType::Triangle, D(Shader, PrimitiveType, Triangle))
        .value("TriangleStrip", PrimitiveType::TriangleStrip, D(Shader, PrimitiveType, TriangleStrip));

    auto renderpass = nb::class_<RenderPass, Object>(m, "RenderPass", D(RenderPass))
        .def(nb::init<std::vector<Object *>, Object *, Object *, Object *, bool>(),
             D(RenderPass, RenderPass), "color_targets"_a, "depth_target"_a = nullptr,
             "stencil_target"_a = nullptr, "blit_target"_a = nullptr, "clear"_a = true)
        .def(nb_method(RenderPass, set_clear_color), D(RenderPass, set_clear_color))
        .def(nb_method(RenderPass, clear_color), D(RenderPass, clear_color))
        .def(nb_method(RenderPass, set_clear_depth), D(RenderPass, set_clear_depth))
        .def(nb_method(RenderPass, clear_depth), D(RenderPass, clear_depth))
        .def(nb_method(RenderPass, set_clear_stencil), D(RenderPass, set_clear_stencil))
        .def(nb_method(RenderPass, clear_stencil), D(RenderPass, clear_stencil))
        .def(nb_method(RenderPass, set_viewport), D(RenderPass, set_viewport), "offset"_a, "size"_a)
        .def(nb_method(RenderPass, viewport), D(RenderPass, viewport))
        .def(nb_method(RenderPass, set_depth_test), D(RenderPass, set_depth_test), "depth_test"_a, "depth_write"_a)
        .def(nb_method(RenderPass, depth_test), D(RenderPass, depth_test))
        .def(nb_method(RenderPass, set_cull_mode), D(RenderPass, set_cull_mode))
        .def(nb_method(RenderPass, cull_mode), D(RenderPass, cull_mode))
        .def(nb_method(RenderPass, begin), D(RenderPass, begin))
        .def(nb_method(RenderPass, end), D(RenderPass, end))
        .def(nb_method(RenderPass, resize), D(RenderPass, resize))
        .def(nb_method(RenderPass, blit_to), D(RenderPass, blit_to),
             "src_offset"_a, "src_size"_a, "dst"_a, "dst_offset"_a)
        .def("__enter__", &RenderPass::begin)
        .def("__exit__", [](RenderPass &rp, nb::handle, nb::handle, nb::handle) { rp.end(); },
             "type"_a.none(), "value"_a.none(), "traceback"_a.none())
#if defined(NANOGUI_USE_OPENGL) || defined(NANOGUI_USE_GLES)
        .def(nb_method(RenderPass, framebuffer_handle))
#elif defined(NANOGUI_USE_METAL)
        .def(nb_method(RenderPass, command_encoder))
#endif
        ;

    nb::enum_<CullMode>(renderpass, "CullMode", D(RenderPass, CullMode))
        .value("Disabled", CullMode::Disabled, D(RenderPass, CullMode, Disabled))
        .value("Front", CullMode::Front, D(RenderPass, CullMode, Front))
        .value("Back", CullMode::Back, D(RenderPass, CullMode, Back));

    nb::enum_<DepthTest>(renderpass, "DepthTest", D(RenderPass, DepthTest))
        .value("Never", DepthTest::Never, D(RenderPass, DepthTest, Never))
        .value("Less", DepthTest::Less, D(RenderPass, DepthTest, Less))
        .value("Equal", DepthTest::Equal, D(RenderPass, DepthTest, Equal))
        .value("LessEqual", DepthTest::LessEqual, D(RenderPass, DepthTest, LessEqual))
        .value("Greater", DepthTest::Greater, D(RenderPass, DepthTest, Greater))
        .value("NotEqual", DepthTest::NotEqual, D(RenderPass, DepthTest, NotEqual))
        .value("GreaterEqual", DepthTest::GreaterEqual, D(RenderPass, DepthTest, GreaterEqual))
        .value("Always", DepthTest::Always, D(RenderPass, DepthTest, Always));
}
