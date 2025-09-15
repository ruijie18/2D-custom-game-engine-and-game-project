R"(#version 450 core

in vec2 TexCoord;                     // Texture coordinates from the vertex shader

uniform float u_Alpha;                // Controls transparency
uniform sampler2D texture1;           // The texture sampler
uniform vec3 tintColor;               // Uniform to tint the texture
uniform vec2 uvScale;                 // UV scale for animation or scaling
uniform vec2 uvOffset;                // UV offset for animation frame selection
uniform int flipTexture;               // Flag to determine if texture should be flipped

out vec4 FragColor;                   // Output color

void main()
{
    vec2 modifiedTexCoord = TexCoord;

    // Apply UV scaling first (for spritesheet animations)
    modifiedTexCoord = modifiedTexCoord * uvScale + uvOffset;

    // If flipTexture is enabled, flip the texture horizontally
    if (flipTexture == 1) {
        modifiedTexCoord.x = 1.0 - modifiedTexCoord.x;
    }

    // Sample the texture
    vec4 sampledTexture = texture(texture1, modifiedTexCoord);

    // Apply tint and alpha
    FragColor = vec4(sampledTexture.rgb * tintColor, sampledTexture.a * u_Alpha);
}


)"