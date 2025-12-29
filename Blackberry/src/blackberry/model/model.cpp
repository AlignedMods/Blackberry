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
        uint8_t* buffer = (uint8_t*)view->buffer->data;
    
        size_t offset = view->offset + accessor->offset;
        return buffer + offset;
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

    Model Model::Create(const FS::Path& path) {
        Model model;

        if (path.Extension() == ".glb" || path.Extension() == ".gltf") { // We are loading a glTF
            cgltf_options options{};
            cgltf_data* data = nullptr;

            std::string strPath = path.String();

            cgltf_result result = cgltf_parse_file(&options, strPath.c_str(), &data);
            if (result != cgltf_result_success) {
                BL_CORE_ERROR("Failed to load mesh from path: {}", path.String());
                return model;
            }

            result = cgltf_load_buffers(&options, data, strPath.c_str());
            if (result != cgltf_result_success) {
                BL_CORE_ERROR("Failed to load mesh buffers from path: {}", path.String());
                cgltf_free(data);
                return model;
            }

            // load all meshes in model
            for (cgltf_size mi = 0; mi < data->meshes_count; ++mi) {
                cgltf_mesh& rawMesh = data->meshes[mi];
                Mesh mesh;
                
                for (cgltf_size pi = 0; pi < rawMesh.primitives_count; ++pi) {
                    cgltf_primitive& primitive = rawMesh.primitives[pi];
                    
                    // Find position attributes
                    cgltf_accessor* posAccessor = nullptr;

                    for (cgltf_size ai = 0; ai < primitive.attributes_count; ++ai) {
                        const cgltf_attribute& attr = primitive.attributes[ai];
                        if (attr.type == cgltf_attribute_type_position) {
                            posAccessor = attr.data;
                            break;
                        }
                    }

                    if (posAccessor) {
                        f32* rawPositions = reinterpret_cast<f32*>(GLTF_GetAccessorData(posAccessor));
                        u32 count = posAccessor->count;
                        u32 stride = posAccessor->stride;

                        // resize to prevent loads of heap allocations
                        mesh.Positions.resize(mesh.Positions.size() + count);

                        for (u32 i = 0; i < count; ++i) {
                            f32* p = reinterpret_cast<f32*>(reinterpret_cast<u8*>(rawPositions) + i * stride);
                            
                            mesh.Positions[i] = BlVec3(p[0], p[1], p[2]);
                        }
                    }

                    // Find index attributes (if they exist)
                    cgltf_accessor* indexAccessor = nullptr;
                    indexAccessor = primitive.indices;

                    if (indexAccessor) {
                        u8* rawIndices = reinterpret_cast<u8*>(GLTF_GetAccessorData(indexAccessor));
                        u32 count = indexAccessor->count;

                        // allocate memory ahead of time
                        mesh.Indices.resize(mesh.Indices.size() + count);

                        for (u32 i = 0; i < count; ++i) {
                            u32 index = 0;

                            switch (indexAccessor->component_type) {
                                case cgltf_component_type_r_16u:
                                    index = (reinterpret_cast<u16*>(rawIndices))[i];
                                    break;

                                case cgltf_component_type_r_32u:
                                    index = (reinterpret_cast<u32*>(rawIndices))[i];
                                    break;

                                case cgltf_component_type_r_8u:
                                    index = (reinterpret_cast<u8*>(rawIndices))[i];
                                    break;

                                default:
                                    BL_CORE_ERROR("Unknown glTF index type!");
                                    break;
                            }

                            mesh.Indices[i] = index;
                        }
                    }

                    // find normals
                    cgltf_accessor* normalAccessor = nullptr;

                    for (cgltf_size ai = 0; ai < primitive.attributes_count; ++ai) {
                        const cgltf_attribute& attr = primitive.attributes[ai];
                        if (attr.type == cgltf_attribute_type_normal) {
                            normalAccessor = attr.data;
                            break;
                        }
                    }

                    if (normalAccessor) {
                        f32* rawNormals = reinterpret_cast<f32*>(GLTF_GetAccessorData(normalAccessor));
                        u32 count = normalAccessor->count;
                        u32 stride = normalAccessor->stride;

                        // resize to prevent loads of heap allocations
                        mesh.Normals.resize(mesh.Normals.size() + count);

                        for (u32 i = 0; i < count; ++i) {
                            f32* p = reinterpret_cast<f32*>(reinterpret_cast<u8*>(rawNormals) + i * stride);
                            
                            mesh.Normals[i] = BlVec3(p[0], p[1], p[2]);
                        }
                    }

                    // find tex coords
                    cgltf_accessor* texCoordAccessor = nullptr;
                    
                    for (cgltf_size ai = 0; ai < primitive.attributes_count; ++ai) {
                        const cgltf_attribute& attr = primitive.attributes[ai];
                        if (attr.type == cgltf_attribute_type_texcoord) {
                            texCoordAccessor = attr.data;
                            break;
                        }
                    }

                    if (texCoordAccessor) {
                        f32* rawTexCoords = reinterpret_cast<f32*>(GLTF_GetAccessorData(texCoordAccessor));
                        u32 count = texCoordAccessor->count;
                        u32 stride = texCoordAccessor->stride;

                        // resize to prevent loads of heap allocations
                        mesh.TexCoords.resize(mesh.TexCoords.size() + count);

                        for (u32 i = 0; i < count; ++i) {
                            f32* p = reinterpret_cast<f32*>(reinterpret_cast<u8*>(rawTexCoords) + i * stride);
                            
                            mesh.TexCoords[i] = BlVec2(p[0], p[1]);
                        }
                    }

                    // find materials
                    cgltf_material* material = primitive.material;

                    if (material) {
                        if (material->has_pbr_metallic_roughness) {
                            Material mat = Material::Create();

                            auto pbr = material->pbr_metallic_roughness;

                            if (pbr.base_color_texture.texture) { // Contains albedo texture
                                mat.AlbedoTexture = GLTF_GetTexture(pbr.base_color_texture);
                                mat.UseAlbedoTexture = true;
                            } else {
                                mat.AlbedoColor = BlVec4(pbr.base_color_factor[0], pbr.base_color_factor[1], pbr.base_color_factor[2], 1.0f);
                            }

                            if (pbr.metallic_roughness_texture.texture) { // Contains metallic/roughness texture
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

                                for (u32 x = 0; x < imRoughness->Width; x++) {
                                    for (u32 y = 0; y < imRoughness->Height; y++) {
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
                            } else {
                                mat.MetallicFactor = pbr.metallic_factor;
                                mat.RoughnessFactor = pbr.roughness_factor;
                            }

                            mat.AOFactor = 1.0f;

                            mesh.MeshMaterial = mat;
                            mesh.HasMeshMaterial = true;
                        }
                    }
                }

                model.Meshes.push_back(mesh);
                model.MeshCount++;
            } // mesh loop

            cgltf_free(data);
        } // if statement

        return model;
    }

} // namespace Blackberry