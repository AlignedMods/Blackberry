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

        std::array<f32, 3> ambient = j.at("Ambient");
        std::array<f32, 3> diffuse = j.at("Diffuse");
        std::array<f32, 3> specular = j.at("Specular");
        f32 shininess = j.at("Shininess");

        mat.Ambient = BlVec3(ambient[0], ambient[1], ambient[2]);
        mat.Diffuse = BlVec3(diffuse[0], diffuse[1], diffuse[2]);
        mat.Specular = BlVec3(specular[0], specular[1], specular[2]);
        mat.Shininess = shininess;

        mat.ID = s_CurrentID;
        s_CurrentID++;

        return mat;
    }

    void Material::Save(Material& mat, const std::filesystem::path& path) {
        json j;

        j["Ambient"] = { mat.Ambient.x, mat.Ambient.y, mat.Ambient.z };
        j["Diffuse"] = { mat.Diffuse.x, mat.Diffuse.y, mat.Diffuse.z };
        j["Specular"] = { mat.Specular.x, mat.Specular.y, mat.Specular.z };
        j["Shininess"] = mat.Shininess;

        std::ofstream file(path);
        file << j.dump(4);
    }

} // namespace Blackberry