#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/scene/scene.hpp"
#include "blackberry/assets/asset_manager.hpp"
#include "blackberry/core/path.hpp"

#include <string>
#include <memory>

namespace Blackberry {

    struct ProjectScene {
        FS::Path Path;
        Scene Scene;
    };

    struct ProjectSpecification {
        std::string Name;

        FS::Path AssetPath;
        FS::Path AssetRegistry;
        FS::Path StartScene;

        std::vector<ProjectScene> Scenes;
    };

    class Project {
    public:
        static void Load(const FS::Path& path);
        static void New();

        static void Save();

        static Scene& LoadScene(const FS::Path& path);
        static void SaveScene(Scene& scene, const FS::Path& path);

        static AssetManager& LoadAssetRegistry(const FS::Path& path);
        static void SaveAssetRegistry(const FS::Path& path);

        static FS::Path GetProjectPath();

        static FS::Path GetAssetDirecory();
        static FS::Path GetAssetPath(const FS::Path& path);

        static ProjectScene& GetStartScene();
        static std::vector<ProjectScene>& GetScenes();

        static AssetManager& GetAssetManager();

    private:
        FS::Path m_ProjectDirectory;
        FS::Path m_ProjectPath;
        AssetManager m_AssetManager;
        ProjectSpecification m_Specification;

        static inline std::shared_ptr<Project> s_ActiveProject;
    };

} // namespace Blackberry