#include "blackberry/model/material.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/project/project.hpp"

#include "json.hpp"
using json = nlohmann::json;

namespace Blackberry {

    static u32 s_CurrentID = 1;

    Material Material::Create(const std::filesystem::path& path) {
        Material mat;

        std::string contents = ReadEntireFile(path);
        json j = json::parse(contents);

        std::string albedoPath = j.at("Albedo");
        std::string metallicPath = j.at("Metallic");
        std::string roughnessPath = j.at("Roughness");
        std::string aoPath = j.at("AO");

        mat.Albedo = Texture2D::Create(Project::GetAssetPath(albedoPath));
        mat.Metallic = Texture2D::Create(Project::GetAssetPath(metallicPath));
        mat.Roughness = Texture2D::Create(Project::GetAssetPath(roughnessPath));
        mat.AO = Texture2D::Create(Project::GetAssetPath(aoPath));

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