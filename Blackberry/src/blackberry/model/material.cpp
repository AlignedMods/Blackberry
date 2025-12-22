#include "blackberry/model/material.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/project/project.hpp"
#include "blackberry/core/yaml_utils.hpp"

namespace Blackberry {

    static u32 s_CurrentID = 1;

    Material Material::Create() {
        Material mat;

        mat.ID = s_CurrentID;
        s_CurrentID++;

        return mat;
    }

    Material Material::Create(const FS::Path& path) {
        Material mat;

        std::string contents = Util::ReadEntireFile(path);
        YAML::Node node = YAML::Load(contents.c_str());

        // albedo
        if (node["Albedo-Texture"]) {
            std::string albedoPath = node["Albedo-Texture"].as<std::string>();
            mat.AlbedoTexture = Texture2D::Create(Project::GetAssetPath(albedoPath));
            mat.AlbedoTexturePath = albedoPath;
            mat.UseAlbedoTexture = true;
        } else if (node["Albedo-Color"]) {
            BlVec4<f32> albedoColor = node["Albedo-Color"].as<BlVec4<f32>>();
            mat.AlbedoColor = albedoColor;
        }

        // metallic
        if (node["Metallic-Texture"]) {
            std::string metallicPath = node["Metallic-Texture"].as<std::string>();
            mat.MetallicTexture = Texture2D::Create(Project::GetAssetPath(metallicPath));
            mat.MetallicTexturePath = metallicPath;
            mat.UseMetallicTexture = true;
        } else if (node["Metallic-Factor"]) {
            f32 metallicFactor = node["Metallic-Factor"].as<f32>();
            mat.MetallicFactor = metallicFactor;
        }

        // roughness
        if (node["Roughness-Texture"]) {
            std::string roughnessPath = node["Roughness-Texture"].as<std::string>();
            mat.RoughnessTexture = Texture2D::Create(Project::GetAssetPath(roughnessPath));
            mat.RoughnessTexturePath = roughnessPath;
            mat.UseRoughnessTexture = true;
        } else if (node["Roughness-Factor"]) {
            f32 roughnessFactor = node["Roughness-Factor"].as<f32>();
            mat.RoughnessFactor = roughnessFactor;
        }

        // AO
        if (node["AO-Texture"]) {
            std::string aoPath = node["AO-Texture"].as<std::string>();
            mat.AOTexture = Texture2D::Create(Project::GetAssetPath(aoPath));
            mat.AOTexturePath = aoPath;
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
        YAML::Emitter out;

        out << YAML::BeginMap;
        
        if (mat.UseAlbedoTexture) {
            out << YAML::Key << "Albedo-Texture" << YAML::Value << mat.AlbedoTexturePath.String();
        } else {
            out << YAML::Key << "Albedo-Color" << YAML::Value << mat.AlbedoColor;
        }

        if (mat.UseMetallicTexture) {
            out << YAML::Key << "Metallic-Texture" << YAML::Value << mat.MetallicTexturePath.String();
        } else {
            out << YAML::Key << "Metallic-Factor" << YAML::Value << mat.MetallicFactor;
        }

        if (mat.UseRoughnessTexture) {
            out << YAML::Key << "Roughness-Texture" << YAML::Value << mat.RoughnessTexturePath.String();
        } else {
            out << YAML::Key << "Roughness-Factor" << YAML::Value << mat.RoughnessFactor;
        }

        if (mat.UseAOTexture) {
            out << YAML::Key << "AO-Texture" << YAML::Value << mat.AOTexturePath.String();
        } else {
            out << YAML::Key << "AO-Factor" << YAML::Value << mat.AOFactor;
        }

        out << YAML::EndMap;

        std::ofstream file(path);
        file << out.c_str();
    }

} // namespace Blackberry