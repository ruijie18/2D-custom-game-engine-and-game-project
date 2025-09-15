R"( #version 450 core

layout(location = 0) in vec2 aPos;        // Vertex position
layout(location = 1) in vec2 aTexCoord;   // Texture coordinates

out vec2 TexCoord;                        // Pass texture coordinates to fragment shader

uniform mat4 projection;                  // Projection matrix
uniform mat4 transform;                   // Transformation matrix (scale, rotate, translate)

void main()
{
    gl_Position = projection * transform * vec4(aPos, 0.0, 1.0);  // Apply both projection and transformation
    TexCoord = aTexCoord;                                          // Pass the texture coordinates
}


)"