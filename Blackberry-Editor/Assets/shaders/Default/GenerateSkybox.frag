#version 460 core

layout (location = 0) in vec3 a_LocalPos;

uniform sampler2D u_EquirectangularMap;

out vec4 o_FragColor;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() {
    vec2 uv = SampleSphericalMap(normalize(a_LocalPos));
    vec3 color = texture(u_EquirectangularMap, uv).rgb;

    o_FragColor = vec4(color, 1.0);
    // o_FragColor = vec4(1.0);
}