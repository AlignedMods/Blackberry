#include "blackberry/model/material.hpp"
#include "blackberry/core/util.hpp"

#include "json.hpp"
using json = nlohmann::json;

namespace Blackberry {

    static u32 s_CurrentID = 1;

    Material Material::Create(const std::filesystem::path& path) {
        Material mat;

        std::string contents = ReadEntireFile(path);
        json j = json::parse(contents);

        std::array<f32, 3> albedo = j.at("Albedo");
        f32 metallic = j.at("Metallic");
        f32 roughness = j.at("Roughness");
        f32 ao = j.at("AO");

        mat.Albedo = BlVec3(albedo[0], albedo[1], albedo[2]);
        mat.Metallic = metallic;
        mat.Roughness = roughness;
        mat.AO = ao;

        mat.ID = s_CurrentID;
        s_CurrentID++;

        return mat;
    }

    void Material::Save(Material& mat, const std::filesystem::path& path) {
        json j;

        j["Albedo"] = { mat.Albedo.x, mat.Albedo.y, mat.Albedo.z };
        j["Metallic"] = mat.Metallic;
        j["Roughness"] = mat.Roughness;
        j["AO"] = mat.AO;

        std::ofstream file(path);
        file << j.dump(4);
    }

} // namespace Blackberry