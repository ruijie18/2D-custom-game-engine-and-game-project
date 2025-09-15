R"( #version 450 core

layout (location = 0) in vec4 vertex; // (x, y, z, w) for position, (u, v) for texture coordinates
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0); // Calculate position in screen space
    TexCoords = vertex.zw; // Pass texture coordinates to fragment shader
}

)"