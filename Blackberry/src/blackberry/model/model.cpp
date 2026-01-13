#include "blackberry/model/model.hpp"
#include "blackberry/core/log.hpp"
#include "blackberry/renderer/texture.hpp"
#include "blackberry/renderer/image.hpp"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#include "stb_image.h" // no need for implementation here because image.cpp has it

namespace Blackberry {

    // gltf helper functions
    static void* GLTF_GetAccessorData(const cgltf_accessor* accessor) {
        cgltf_buffer_view* view = accessor->buffer_view;
        uint8_t* buffer = reinterpret_cast<uint8_t*>(view->buffer->data);
    
        size_t offset = view->offset + accessor->offset;
        return buffer + offset;
    }

    
    template<typename T>
    T GLTF_Read(const cgltf_accessor* accessor, size_t index)
    {
        T out{};
        cgltf_accessor_read_float(accessor, index, reinterpret_cast<float*>(&out), sizeof(T) / sizeof(float));
        return out;
    }

    static Ref<Texture> GLTF_GetTexture(cgltf_texture_view& tex) {
        if (!tex.texture) return CreateRef<Texture>();

        cgltf_buffer_view* view = tex.texture->image->buffer_view;
        u8* data = reinterpret_cast<u8*>(view->buffer->data) + view->offset;
        u32 size = view->size;

        int width, height;
        void* pixels = stbi_load_from_memory(data, size, &width, &height, nullptr, 4);

        Ref<Texture> texture = Texture2D::Create(pixels, width, height, TextureFormat::RGBA8);

        return texture;
    }

    static Ref<Image> GLTF_GetImage(cgltf_image* image) {
        cgltf_buffer_view* view = image->buffer_view;
        u8* data = reinterpret_cast<u8*>(view->buffer->data) + view->offset;
        u32 size = view->size;

        int width, height;
        void* pixels = stbi_load_from_memory(data, size, &width, &height, nullptr, 4);

        Ref<Image> im = Image::Create(pixels, width, height, ImageFormat::RGBA8);

        stbi_image_free(pixels);

        return im;
    }

    static bool LoadFromGLTF(const FS::Path& path, Model& out) {
        cgltf_options options{};
        cgltf_data* data = nullptr;

        std::string strFilename = path.String();
        const char* filename = strFilename.c_str();

        if (cgltf_parse_file(&options, filename, &data) != cgltf_result_success)
            return false;

        if (cgltf_load_buffers(&options, data, filename) != cgltf_result_success)
            return false;

        if (cgltf_validate(data) != cgltf_result_success)
            return false;

        BL_CORE_TRACE("Mesh: {}", path.String());
        BL_CORE_TRACE("    Mesh count: {}", data->meshes_count);
        BL_CORE_TRACE("    Material count: {}", data->materials_count);
        BL_CORE_TRACE("    Image count: {}", data->images_count);

        // Load materials
        out.Materials.reserve(data->materials_count);
        for (u32 i = 0; i < data->materials_count; i++) {
            cgltf_material& gltfMat = data->materials[i];
            Material mat = Material::Create();

            if (gltfMat.has_pbr_metallic_roughness) {
                cgltf_pbr_metallic_roughness pbr = gltfMat.pbr_metallic_roughness;

                mat.AlbedoColor = BlVec4(pbr.base_color_factor[0], pbr.base_color_factor[1], pbr.base_color_factor[2], pbr.base_color_factor[3]);
                if (pbr.base_color_texture.texture) {
                    mat.AlbedoTexture = GLTF_GetTexture(pbr.base_color_texture);
                    mat.UseAlbedoTexture = true;
                }

                mat.MetallicFactor = pbr.metallic_factor;
                mat.RoughnessFactor = pbr.roughness_factor;
                if (pbr.metallic_roughness_texture.texture) {
                    Ref<Image> imMetallicRoughness = GLTF_GetImage(pbr.metallic_roughness_texture.texture->image);

                    // Extract roughness and metallic values from the combined image
                    Ref<Image> imMetallic = CreateRef<Image>();
                    Ref<Image> imRoughness = CreateRef<Image>();
                    
                    imMetallic->Pixels = malloc(imMetallicRoughness->Width * imMetallicRoughness->Height);
                    imRoughness->Pixels = malloc(imMetallicRoughness->Width * imMetallicRoughness->Height);

                    imMetallic->Format = ImageFormat::U8;
                    imRoughness->Format = ImageFormat::U8;

                    imMetallic->Width = imMetallicRoughness->Width;
                    imRoughness->Width = imMetallicRoughness->Width;

                    imMetallic->Height = imMetallicRoughness->Height;
                    imRoughness->Height = imMetallicRoughness->Height;

                    for (u32 y = 0; y < imRoughness->Height; y++) {
                        for (u32 x = 0; x < imRoughness->Width; x++) {
                            BlColor color;
                            color.r = reinterpret_cast<u8*>(imMetallicRoughness->Pixels)[(y * imMetallicRoughness->Height + x) * 4 + 0];
                            color.g = reinterpret_cast<u8*>(imMetallicRoughness->Pixels)[(y * imMetallicRoughness->Height + x) * 4 + 1];
                            color.b = reinterpret_cast<u8*>(imMetallicRoughness->Pixels)[(y * imMetallicRoughness->Height + x) * 4 + 2];
                            color.a = reinterpret_cast<u8*>(imMetallicRoughness->Pixels)[(y * imMetallicRoughness->Height + x) * 4 + 3];

                            reinterpret_cast<u8*>(imMetallic->Pixels)[y * imMetallic->Height + x] = color.g;
                            reinterpret_cast<u8*>(imRoughness->Pixels)[y * imRoughness->Height + x] = color.b;
                        }
                    }

                    mat.MetallicTexture = Texture2D::Create(imMetallic);
                    mat.RoughnessTexture = Texture2D::Create(imRoughness);

                    mat.UseMetallicTexture = true;
                    mat.UseRoughnessTexture = true;
                }
            }

            out.Materials.push_back(mat);
        }

        // Load meshes
        out.Meshes.reserve(data->meshes_count);
        for (u32 i = 0; i < data->nodes_count; i++) {
            cgltf_node& node = data->nodes[i];

            if (!node.mesh) continue;
            cgltf_mesh& gltfMesh = *node.mesh;

            // Get transform
            cgltf_float trans[16]{};
            cgltf_node_transform_local(&node, trans);
            BlMat4 mat = BlMat4(trans[0],  trans[1],  trans[2],  trans[3],
                                trans[4],  trans[5],  trans[6],  trans[7],
                                trans[8],  trans[9],  trans[10], trans[11],
                                trans[12], trans[13], trans[14], trans[15]);

            for (u32 p = 0; p < gltfMesh.primitives_count; p++) {
                cgltf_primitive& prim = gltfMesh.primitives[p];
                Mesh mesh;
                mesh.Transform = mat;

                const cgltf_accessor* position = nullptr;
                const cgltf_accessor* normal = nullptr;
                const cgltf_accessor* texCoord = nullptr;

                for (u32 a = 0; a < prim.attributes_count; a++) {
                    cgltf_attribute& attrib = prim.attributes[a];

                    switch (attrib.type) {
                        case cgltf_attribute_type_position: position = attrib.data; break;
                        case cgltf_attribute_type_normal: normal = attrib.data; break;
                        case cgltf_attribute_type_texcoord: texCoord = attrib.data; break;
                        case cgltf_attribute_type_color: BL_CORE_WARN("Attribute type color not supported"); break;
                        case cgltf_attribute_type_tangent: BL_CORE_WARN("Attribute type tangent not supported"); break;
                        default: BL_ASSERT(false, "To be implemented!"); break;
                    }
                }

                u32 count = position->count;
                mesh.Positions.reserve(count);
                mesh.Normals.reserve(count);
                mesh.TexCoords.reserve(count);
                
                for (u32 v = 0; v < count; v++) {
                    mesh.Positions.push_back(GLTF_Read<BlVec3>(position, v));
                    if (normal) {
                        mesh.Normals.push_back(GLTF_Read<BlVec3>(normal, v));
                    }
                    if (texCoord) {
                        mesh.TexCoords.push_back(GLTF_Read<BlVec2>(texCoord, v));
                    }
                }

                // Indices
                if (prim.indices) {
                    mesh.Indices.reserve(prim.indices->count);
                    for (u32 in = 0; in < prim.indices->count; in++) {
                        mesh.Indices.push_back(static_cast<u32>(cgltf_accessor_read_index(prim.indices, in)));
                    }
                }

                // Material
                if (prim.material) {
                    // Nice little pointer arithmetic am i right
                    mesh.MaterialIndex = static_cast<u32>(prim.material - data->materials);
                }

                out.Meshes.push_back(mesh);
            }
        }

        cgltf_free(data);
        return true;
    }

    Model Model::Create(const FS::Path& path) {
        Model model;

        if (path.Extension() == ".glb" || path.Extension() == ".gltf") { // We are loading a glTF
            if (path.FileName() == "gun.glb") {
                // BL_DEBUGBREAK();
            }
            if (!LoadFromGLTF(path, model)) {
                BL_CORE_WARN("Failed to load gltf model from {}!", path.String());
            }
        }

        return model;
    }

} // namespace Blackberry