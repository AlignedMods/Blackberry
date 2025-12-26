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
        s_ActiveProject->m_Specification.StartScene = startScene;

        s_ActiveProject->m_ProjectPath = path;
    }

    void Project::Save() {
        BL_ASSERT(s_ActiveProject, "No active project!");
        
        // for (auto& scene : s_ActiveProject->m_Specification.Scenes) {
        //     SaveScene(scene.Scene, scene.Path);
        // }

        SaveAssetRegistry(s_ActiveProject->m_Specification.AssetRegistry);
    }

    void Project::SaveScene(Ref<Scene> scene, const FS::Path& path) {
        SceneSerializer serializer(scene);
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

    Ref<Scene> Project::GetStartScene() {
        BL_ASSERT(s_ActiveProject, "No active project!");

        BL_ASSERT(s_ActiveProject->m_AssetManager.ContainsAsset(s_ActiveProject->m_Specification.StartScene), "Project does not contain start scene!");

        Ref<Scene> scene = std::get<Ref<Scene>>(s_ActiveProject->m_AssetManager.GetAssetFromPath(s_ActiveProject->m_Specification.StartScene).Data);
        return scene;
    }

    AssetManager& Project::GetAssetManager() {
        BL_ASSERT(s_ActiveProject, "No active project!");
        return s_ActiveProject->m_AssetManager;
    }

    ProjectSpecification& Project::GetSpecification() {
        BL_ASSERT(s_ActiveProject, "No active project!");
        return s_ActiveProject->m_Specification;
    }

} // namespace Blackberry