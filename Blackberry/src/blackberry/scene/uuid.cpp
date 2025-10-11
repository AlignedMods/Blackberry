#include "uuid.hpp"

#include <random>

namespace Blackberry {

    static std::random_device s_RandomDevice;
	static std::mt19937_64 s_Engine(s_RandomDevice());
	static std::uniform_int_distribution<u64> s_UniformDistribution;

    u64 UUID() {
        return s_UniformDistribution(s_Engine);
    }

} // namespace Blackberry