#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/scene/scene.hpp"
#include "blackberry/assets/asset_manager.hpp"
#include "blackberry/core/path.hpp"

#include <string>
#include <memory>

namespace Blackberry {

    struct ProjectSpecification {
        std::string Name;

        FS::Path AssetPath;
        FS::Path AssetRegistry;
        FS::Path StartScene;
    };

    class Project {
    public:
        static void Load(const FS::Path& path);
        static void New();

        static void Save();

        static void SaveScene(Ref<Scene> scene, const FS::Path& path);

        static AssetManager& LoadAssetRegistry(const FS::Path& path);
        static void SaveAssetRegistry(const FS::Path& path);

        static FS::Path GetProjectPath();

        static FS::Path GetAssetDirecory();
        static FS::Path GetAssetPath(const FS::Path& path);

        static Ref<Scene> GetStartScene();

        static AssetManager& GetAssetManager();

        static ProjectSpecification& GetSpecification();

    private:
        FS::Path m_ProjectDirectory;
        FS::Path m_ProjectPath;
        AssetManager m_AssetManager;
        ProjectSpecification m_Specification;

        static inline std::shared_ptr<Project> s_ActiveProject;
    };

} // namespace Blackberry