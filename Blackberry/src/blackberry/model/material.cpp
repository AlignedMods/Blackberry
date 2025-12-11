#include "blackberry/model/material.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/project/project.hpp"

#include "json.hpp"
using json = nlohmann::json;

namespace Blackberry {

    static u32 s_CurrentID = 1;

    Material Material::Create(const std::filesystem::path& path) {
        Material mat;

        std::string contents = Util::ReadEntireFile(path);
        json j = json::parse(contents);

        // albedo
        if (j.contains("Albedo-Texture")) {
            std::string albedoPath = j.at("Albedo-Texture");
            mat.AlbedoTexture = Texture2D::Create(Project::GetAssetPath(albedoPath));
            mat.UseAlbedoTexture = true;
        } else if (j.contains("Albedo-Color")) {
            std::array<f32, 3> albedoColor = j.at("Albedo-Color");
            mat.AlbedoColor = BlVec4(albedoColor[0], albedoColor[1], albedoColor[2], 1.0f);
        }

        // metallic
        if (j.contains("Metallic-Texture")) {
            std::string metallicPath = j.at("Metallic-Texture");
            mat.MetallicTexture = Texture2D::Create(Project::GetAssetPath(metallicPath));
            mat.UseMetallicTexture = true;
        } else if (j.contains("Metallic-Factor")) {
            f32 metallicFactor = j.at("Metallic-Factor");
            mat.MetallicFactor = metallicFactor;
        }

        // roughness
        if (j.contains("Roughness-Texture")) {
            std::string roughnessPath = j.at("Roughness-Texture");
            mat.RoughnessTexture = Texture2D::Create(Project::GetAssetPath(roughnessPath));
            mat.UseRoughnessTexture = true;
        } else if (j.contains("Roughness-Factor")) {
            f32 roughnessFactor = j.at("Roughness-Factor");
            mat.RoughnessFactor = roughnessFactor;
        }

        // AO
        if (j.contains("AO-Texture")) {
            std::string aoPath = j.at("AO-Texture");
            mat.AOTexture = Texture2D::Create(Project::GetAssetPath(aoPath));
            mat.UseAOTexture = true;
        } else if (j.contains("AO-Factor")) {
            f32 aoFactor = j.at("AO-Factor");
            mat.AOFactor = aoFactor;
        }

        mat.ID = s_CurrentID;
        s_CurrentID++;

        return mat;
    }

    void Material::Save(Material& mat, const std::filesystem::path& path) {
        json j;

        // j["Albedo"] = { mat.Albedo.x, mat.Albedo.y, mat.Albedo.z };
        // j["Metallic"] = mat.Metallic;
        // j["Roughness"] = mat.Roughness;
        // j["AO"] = mat.AO;

        std::ofstream file(path);
        file << j.dump(4);
    }

} // namespace Blackberry