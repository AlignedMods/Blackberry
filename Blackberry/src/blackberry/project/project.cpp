#include "blackberry/project/project.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/scene/scene_serializer.hpp"
#include "blackberry/assets/asset_serializer.hpp"

#include "json.hpp"

namespace Blackberry {

    using json = nlohmann::json;

    void Project::Load(const FS::Path& path) {
        if (!FS::Exists(path)) { return; }

        std::string contents = Util::ReadEntireFile(path);
        json j = json::parse(contents);

        s_ActiveProject = std::make_shared<Project>();

        s_ActiveProject->m_ProjectDirectory = path.ParentPath();
        std::string assetDir = j.at("AssetsDirectory");
        s_ActiveProject->m_Specification.AssetPath = s_ActiveProject->m_ProjectDirectory / assetDir;

        std::string assetRegistry = j.at("AssetRegistry");
        s_ActiveProject->m_Specification.AssetRegistry = s_ActiveProject->m_Specification.AssetPath / assetRegistry;
        LoadAssetRegistry(s_ActiveProject->m_Specification.AssetPath / assetRegistry);
    
        std::string startScene = j.at("StartScene");
        FS::Path scenePath = s_ActiveProject->m_Specification.AssetPath / startScene;
        LoadScene(scenePath);

        s_ActiveProject->m_ProjectPath = path;
    }

    void Project::Save() {
        BL_ASSERT(s_ActiveProject, "No active project!");
        
        for (auto& scene : s_ActiveProject->m_Specification.Scenes) {
            SaveScene(scene.Scene, scene.Path);
        }

        SaveAssetRegistry(s_ActiveProject->m_Specification.AssetRegistry);
    }

    Scene& Project::LoadScene(const FS::Path& path) {
        BL_ASSERT(s_ActiveProject, "No active project!");
        ProjectScene scene;
        scene.Path = path;

        SceneSerializer serializer(&scene.Scene);
        serializer.Deserialize(path);

        return s_ActiveProject->m_Specification.Scenes.emplace_back(scene).Scene;
    }

    void Project::SaveScene(Scene& scene, const FS::Path& path) {
        SceneSerializer serializer(&scene);
        serializer.Serialize(path);
    }

    AssetManager& Project::LoadAssetRegistry(const FS::Path& path) {
        BL_ASSERT(s_ActiveProject, "No active project!");
        AssetSerializer serializer(&s_ActiveProject->m_AssetManager);
        serializer.Deserialize(path);

        return s_ActiveProject->m_AssetManager;
    }

    void Project::SaveAssetRegistry(const FS::Path& path) {
        BL_ASSERT(s_ActiveProject, "No active project!");
        AssetSerializer serializer(&s_ActiveProject->m_AssetManager);
        serializer.Serialize(path);
    }

    FS::Path Project::GetProjectPath() {
        BL_ASSERT(s_ActiveProject, "No active project!");
        return s_ActiveProject->m_ProjectPath;
    }

    FS::Path Project::GetAssetDirecory() {
        BL_ASSERT(s_ActiveProject, "No active project!");
        return s_ActiveProject->m_Specification.AssetPath;
    }

    FS::Path Project::GetAssetPath(const FS::Path& path) {
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

    AssetManager& Project::GetAssetManager() {
        BL_ASSERT(s_ActiveProject, "No active project!");
        return s_ActiveProject->m_AssetManager;
    }

} // namespace Blackberry