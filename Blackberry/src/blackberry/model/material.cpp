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

        u64 diffuse = j.at("Diffuse");
        u64 specular = j.at("Specular");
        f32 shininess = j.at("Shininess");

        mat.Diffuse = diffuse;
        mat.Specular = specular;
        mat.Shininess = shininess;

        mat.ID = s_CurrentID;
        s_CurrentID++;

        return mat;
    }

    void Material::Save(Material& mat, const std::filesystem::path& path) {
        json j;

        j["Diffuse"] = mat.Diffuse;
        j["Specular"] = mat.Specular;
        j["Shininess"] = mat.Shininess;

        std::ofstream file(path);
        file << j.dump(4);
    }

} // namespace Blackberry