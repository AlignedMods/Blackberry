#pragma once

#include "blackberry/core/types.hpp"

#include "yaml-cpp/yaml.h"

namespace YAML {
    
    template <>
    struct convert<BlVec3<f32>> {
        static Node encode(const BlVec3<f32>& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }

        static bool decode(const Node& node, BlVec3<f32>& rhs) {
            if (!node.IsSequence() || node.size() != 3) {
                return false;
            }

            rhs.x = node[0].as<f32>();
            rhs.y = node[1].as<f32>();
            rhs.z = node[2].as<f32>();

            return true;
        }
    };

    template <>
    struct convert<BlVec4<f32>> {
        static Node encode(const BlVec4<f32>& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            return node;
        }

        static bool decode(const Node& node, BlVec4<f32>& rhs) {
            if (!node.IsSequence() || node.size() != 4) {
                return false;
            }

            rhs.x = node[0].as<f32>();
            rhs.y = node[1].as<f32>();
            rhs.z = node[2].as<f32>();
            rhs.w = node[3].as<f32>();

            return true;
        }
    };

} // namespace YAML

inline YAML::Emitter& operator<<(YAML::Emitter& out, BlVec3<f32> vec) {
    out << YAML::Flow << YAML::BeginSeq;
    out << vec.x << vec.y << vec.z;
    out << YAML::EndSeq;

    return out;
}

inline YAML::Emitter& operator<<(YAML::Emitter& out, BlVec4<f32> vec) {
    out << YAML::Flow << YAML::BeginSeq;
    out << vec.x << vec.y << vec.z << vec.w;
    out << YAML::EndSeq;

    return out;
}