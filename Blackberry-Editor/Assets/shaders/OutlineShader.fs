#version 460 core

layout (location = 0) in vec2 a_TexCoord;

out vec4 o_FragColor;

uniform sampler2D u_Mask;
uniform vec2 u_TexelSize;      // 1.0 / framebufferSize
uniform float u_Thickness;     // thickness in pixels
uniform vec3 u_OutlineColor;   // e.g. vec3(1.0, 1.0, 0.0)

void main()
{
    // Sobel kernels
    float gx[9] = float[9](-1,0,1, -2,0,2, -1,0,1);
    float gy[9] = float[9](-1,-2,-1, 0,0,0, 1,2,1);

    float sampleVals[9];
    int i = 0;
    for(int y=-1; y<=1; y++) {
        for(int x=-1; x<=1; x++) {
            vec2 offset = vec2(x, y) * u_TexelSize * u_Thickness;
            sampleVals[i++] = texture(u_Mask, a_TexCoord + offset).r;
        }
    }

    float gxSum = 0.0;
    float gySum = 0.0;
    for(int k=0; k<9; k++) {
        gxSum += gx[k] * sampleVals[k];
        gySum += gy[k] * sampleVals[k];
    }

    float edge = sqrt(gxSum*gxSum + gySum*gySum);
    edge = smoothstep(0.05, 0.2, edge); // tweak thresholds

    o_FragColor = vec4(u_OutlineColor, edge);

    // o_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}