#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/scene/scene.hpp"
#include "blackberry/assets/asset_manager.hpp"

#include <string>
#include <filesystem>
#include <memory>

namespace Blackberry {

    struct ProjectScene {
        std::filesystem::path Path;
        Scene Scene;
    };

    struct ProjectSpecification {
        std::string Name;

        std::filesystem::path AssetPath;
        std::filesystem::path AssetRegistry;
        std::filesystem::path StartScene;

        std::vector<ProjectScene> Scenes;
    };

    class Project {
    public:
        static void Load(const std::filesystem::path& path);
        static void New();

        static void Save();

        static Scene& LoadScene(const std::filesystem::path& path);
        static void SaveScene(Scene& scene, const std::filesystem::path& path);

        static AssetManager& LoadAssetRegistry(const std::filesystem::path& path);
        static void SaveAssetRegistry(const std::filesystem::path& path);

        static std::filesystem::path GetProjectPath();

        static std::filesystem::path GetAssetDirecory();
        static std::filesystem::path GetAssetPath(const std::filesystem::path& path);

        static ProjectScene& GetStartScene();
        static std::vector<ProjectScene>& GetScenes();

        static AssetManager& GetAssetManager();

    private:
        std::filesystem::path m_ProjectDirectory;
        std::filesystem::path m_ProjectPath;
        AssetManager m_AssetManager;
        ProjectSpecification m_Specification;

        static inline std::shared_ptr<Project> s_ActiveProject;
    };

} // namespace Blackberry