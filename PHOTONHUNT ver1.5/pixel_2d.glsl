#version 330 core

in vec4 vColor;
in vec2 vUV;
in float vHasTex;

uniform sampler2D uTexture;

out vec4 FragColor;

void main()
{
    vec4 baseColor = vColor;
    baseColor = vColor;
    if (vHasTex > 0.5)
    {
        vec4 texColor = texture(uTexture, vUV);
        baseColor *= texColor;
    }

    FragColor = baseColor;
}
