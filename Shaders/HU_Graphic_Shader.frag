R"( #version 450 core

in vec2 TexCoord;                     // Texture coordinates from the vertex shader

uniform float u_Alpha; // Controls transparency
uniform sampler2D texture1;            // The texture sampler
uniform bool useTexture;               // Flag to toggle between texture and color
uniform vec3 shapeColor;               // Color for non-textured objects

//Uniforms for UV animation (spritesheet)
uniform vec2 uvOffset = vec2(0.0, 0.0);  // UV offset for the current frame
uniform vec2 uvScale = vec2(1.0, 1.0);   // UV scale per frame


out vec4 FragColor;                    // Output color

void main()
{
    if (useTexture)                    // If texture is enabled
    {
        vec2 animatedCoords = TexCoord * uvScale + uvOffset;
	vec4 sampledTexture = texture(texture1, animatedCoords); //sample the texture
       FragColor = vec4(sampledTexture.rgb, sampledTexture.a * u_Alpha); // Apply transparency
    }
    else
    {
        FragColor = vec4(shapeColor, u_Alpha);       // Use a solid color
    }
}


)"