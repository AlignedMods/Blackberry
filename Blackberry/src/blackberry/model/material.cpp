#include "blackberry/model/material.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/project/project.hpp"

#include "yaml-cpp/yaml.h"

namespace YAML {
    
    template <>
    struct convert<BlVec3<f32>> {
        static Node encode(const BlVec3<f32>& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }

        static bool decode(const Node& node, BlVec3<f32>& rhs) {
            if (!node.IsSequence() || node.size() != 3) {
                return false;
            }

            rhs.x = node[0].as<f32>();
            rhs.y = node[1].as<f32>();
            rhs.z = node[2].as<f32>();

            return true;
        }
    };

} // namespace YAML

namespace Blackberry {

    static u32 s_CurrentID = 1;

    Material Material::Create(const FS::Path& path) {
        Material mat;

        std::string contents = Util::ReadEntireFile(path);
        YAML::Node node = YAML::Load(contents.c_str());

        // albedo
        if (node["Albedo-Texture"]) {
            std::string albedoPath = node["Albedo-Texture"].as<std::string>();
            mat.AlbedoTexture = Texture2D::Create(Project::GetAssetPath(albedoPath));
            mat.UseAlbedoTexture = true;
        } else if (node["Albedo-Color"]) {
            BlVec3<f32> albedoColor = node["Albedo-Color"].as<BlVec3<f32>>();
            mat.AlbedoColor = BlVec4(albedoColor.x, albedoColor.y, albedoColor.z, 1.0f);
        }

        // metallic
        if (node["Metallic-Texture"]) {
            std::string metallicPath = node["Metallic-Texture"].as<std::string>();
            mat.MetallicTexture = Texture2D::Create(Project::GetAssetPath(metallicPath));
            mat.UseMetallicTexture = true;
        } else if (node["Metallic-Factor"]) {
            f32 metallicFactor = node["Metallic-Factor"].as<f32>();
            mat.MetallicFactor = metallicFactor;
        }

        // roughness
        if (node["Roughness-Texture"]) {
            std::string roughnessPath = node["Roughness-Texture"].as<std::string>();
            mat.RoughnessTexture = Texture2D::Create(Project::GetAssetPath(roughnessPath));
            mat.UseRoughnessTexture = true;
        } else if (node["Roughness-Factor"]) {
            f32 roughnessFactor = node["Roughness-Factor"].as<f32>();
            mat.RoughnessFactor = roughnessFactor;
        }

        // AO
        if (node["AO-Texture"]) {
            std::string aoPath = node["AO-Texture"].as<std::string>();
            mat.AOTexture = Texture2D::Create(Project::GetAssetPath(aoPath));
            mat.UseAOTexture = true;
        } else if (node["AO-Factor"]) {
            f32 aoFactor = node["AO-Factor"].as<f32>();
            mat.AOFactor = aoFactor;
        }

        mat.ID = s_CurrentID;
        s_CurrentID++;

        return mat;
    }

    void Material::Save(Material& mat, const FS::Path& path) {
        // j["Albedo"] = { mat.Albedo.x, mat.Albedo.y, mat.Albedo.z };
        // j["Metallic"] = mat.Metallic;
        // j["Roughness"] = mat.Roughness;
        // j["AO"] = mat.AO;

        std::ofstream file(path);
    }

} // namespace Blackberry