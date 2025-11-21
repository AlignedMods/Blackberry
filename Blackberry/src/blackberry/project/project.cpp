#include "blackberry/project/project.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/scene/serializer.hpp"

#include "json.hpp"

namespace Blackberry {

    using json = nlohmann::json;

    void Project::Load(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) { return; }

        std::string contents = ReadEntireFile(path);
        json j = json::parse(contents);

        s_ActiveProject = std::make_shared<Project>();

        s_ActiveProject->m_ProjectDirectory = path.parent_path();
        std::string assetDir = j.at("AssetsDirectory");
        s_ActiveProject->m_Specification.AssetPath = s_ActiveProject->m_ProjectDirectory / assetDir;
    
        std::string startScene = j.at("StartScene");
        std::filesystem::path scenePath = s_ActiveProject->m_Specification.AssetPath / startScene;
        LoadScene(scenePath);

        s_ActiveProject->m_ProjectPath = path;
    }

    void Project::Save() {
        BL_ASSERT(s_ActiveProject, "No active project!");
        
        for (auto& scene : s_ActiveProject->m_Specification.Scenes) {
            SaveScene(scene.Scene, scene.Path);
        }
    }

    Scene& Project::LoadScene(const std::filesystem::path& path) {
        BL_ASSERT(s_ActiveProject, "No active project!");
        ProjectScene scene;
        scene.Path = path;

        SceneSerializer serializer(&scene.Scene, s_ActiveProject->m_Specification.AssetPath);
        serializer.Deserialize(path);

        return s_ActiveProject->m_Specification.Scenes.emplace_back(scene).Scene;
    }

    void Project::SaveScene(Scene& scene, const std::filesystem::path& path) {
        SceneSerializer serializer(&scene, GetAssetDirecory());
        serializer.Serialize(path);
    }

    std::filesystem::path Project::GetProjectPath() {
        BL_ASSERT(s_ActiveProject, "No active project!");
        return s_ActiveProject->m_ProjectPath;
    }

    std::filesystem::path Project::GetAssetDirecory() {
        BL_ASSERT(s_ActiveProject, "No active project!");
        return s_ActiveProject->m_Specification.AssetPath;
    }

    std::filesystem::path Project::GetAssetPath(const std::filesystem::path& path) {
        BL_ASSERT(s_ActiveProject, "No active project!");
        return GetAssetDirecory() / path;
    }

    ProjectScene& Project::GetStartScene() {
        BL_ASSERT(s_ActiveProject, "No active project!");
        return s_ActiveProject->m_Specification.Scenes.front();
    }

    std::vector<ProjectScene>& Project::GetScenes() {
        BL_ASSERT(s_ActiveProject, "No active project!");
        return s_ActiveProject->m_Specification.Scenes;
    }

} // namespace Blackberry