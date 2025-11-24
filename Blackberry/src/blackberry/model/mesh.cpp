#include "blackberry/model/mesh.hpp"
#include "blackberry/core/log.hpp"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#include "stb_image.h" // no need for implementation here because image.cpp has it

namespace Blackberry {

    static void* GetAccessorData(const cgltf_accessor* accessor) {
        cgltf_buffer_view* view = accessor->buffer_view;
        uint8_t* buffer = (uint8_t*)view->buffer->data;
    
        size_t offset = view->offset + accessor->offset;
        return buffer + offset;
    }

    // (aligned) TODO: Improve the loading here to not blindly assume everything
    Mesh Mesh::Create(const std::filesystem::path& path) {
        Mesh mesh;

        cgltf_options options{};
        cgltf_data* data = nullptr;

        cgltf_result result = cgltf_parse_file(&options, path.string().c_str(), &data);
        if (result != cgltf_result_success) {
            BL_CORE_ERROR("Failed to load mesh from path: {}", path.string());
            return mesh;
        }

        result = cgltf_load_buffers(&options, data, path.string().c_str());
        if (result != cgltf_result_success) {
            BL_CORE_ERROR("Failed to load mesh buffers from path: {}", path.string());
            cgltf_free(data);
            return mesh;
        }

        for (cgltf_size mi = 0; mi < data->meshes_count; ++mi) {
            cgltf_mesh& rawMesh = data->meshes[mi];
            
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
                    f32* rawPositions = reinterpret_cast<f32*>(GetAccessorData(posAccessor));
                    u32 count = posAccessor->count;
                    u32 stride = posAccessor->stride;

                    BL_CORE_INFO("Vertex count: {}", count);

                    // resize to prevent loads of heap allocations
                    mesh.Positions.resize(count);

                    for (u32 i = 0; i < count; ++i) {
                        f32* p = reinterpret_cast<f32*>(reinterpret_cast<u8*>(rawPositions) + i * stride);
                        
                        BL_CORE_INFO("Vertex position: {}, {}, {}", p[0], p[1], p[2]);
                        mesh.Positions[i] = BlVec3<f32>(p[0], p[1], p[2]);
                    }
                }

                // Find index attributes (if they exist)
                cgltf_accessor* indexAccessor = nullptr;
                indexAccessor = primitive.indices;

                if (indexAccessor) {
                    u8* rawIndices = reinterpret_cast<u8*>(GetAccessorData(indexAccessor));
                    u32 count = indexAccessor->count;

                    BL_CORE_INFO("Index count: {}", count);

                    // allocate memory ahead of time
                    mesh.Indices.resize(count);

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

                        BL_CORE_INFO("Index: {}", index);
                        mesh.Indices[i] = index;
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
                    f32* rawTexCoords = reinterpret_cast<f32*>(GetAccessorData(texCoordAccessor));
                    u32 count = texCoordAccessor->count;
                    u32 stride = texCoordAccessor->stride;

                    BL_CORE_INFO("TexCoord count: {}", count);

                    // resize to prevent loads of heap allocations
                    mesh.TexCoords.resize(count);

                    for (u32 i = 0; i < count; ++i) {
                        f32* p = reinterpret_cast<f32*>(reinterpret_cast<u8*>(rawTexCoords) + i * stride);
                        
                        BL_CORE_INFO("TexCoord: {}, {}", p[0], p[1]);
                        mesh.TexCoords[i] = BlVec2<f32>(p[0], p[1]);
                    }
                }

                // find textures
                cgltf_buffer_view* texBufferView = data->textures[0].image->buffer_view;
                void* imageBytes = reinterpret_cast<u8*>(texBufferView->buffer->data) + texBufferView->offset;
                int width, height, channels;

                u8* imageData = stbi_load_from_memory(reinterpret_cast<u8*>(imageBytes), texBufferView->buffer->size, &width, &height, &channels, 4);

                mesh.Texture = Texture2D::Create(imageData, width, height, ImageFormat::RGBA8);
                stbi_image_free(imageData);
            }
        }

        cgltf_free(data);

        return mesh;
    }

} // namespace Blackberry