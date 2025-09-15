R"( #version 450 core

layout(location = 0) in vec2 aPos;        // Vertex position
layout(location = 1) in vec2 aTexCoord;   // Texture coordinates

out vec2 TexCoord;                        // Pass texture coordinates to fragment shader

uniform mat4 projection;                  // Projection matrix
uniform mat4 view;                        // View matrix (camera)
uniform mat4 transform;                   // Model transformation matrix (scale, rotate, translate)

void main()
{
    // Combine projection, view, and transform (model) into MVP matrix
    gl_Position = projection * view * transform * vec4(aPos, 0.0, 1.0); 
    TexCoord = aTexCoord;  // Pass the texture coordinates
}


)"