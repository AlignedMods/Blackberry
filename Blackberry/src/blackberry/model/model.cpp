#include "blackberry/model/model.hpp"
#include "blackberry/core/log.hpp"

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

    Model Model::Create(const std::filesystem::path& path) {
        Model model;

        if (path.extension().string() == ".glb" || path.extension() == ".gltf") { // We are loading a glTF
            cgltf_options options{};
            cgltf_data* data = nullptr;

            cgltf_result result = cgltf_parse_file(&options, path.string().c_str(), &data);
            if (result != cgltf_result_success) {
                BL_CORE_ERROR("Failed to load mesh from path: {}", path.string());
                return model;
            }

            result = cgltf_load_buffers(&options, data, path.string().c_str());
            if (result != cgltf_result_success) {
                BL_CORE_ERROR("Failed to load mesh buffers from path: {}", path.string());
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
                            
                            mesh.Positions[i] = BlVec3<f32>(p[0], p[1], p[2]);
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
                            
                            mesh.Normals[i] = BlVec3<f32>(p[0], p[1], p[2]);
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
                            
                            mesh.TexCoords[i] = BlVec2<f32>(p[0], p[1]);
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