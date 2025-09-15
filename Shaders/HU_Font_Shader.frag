R"( #version 450 core

in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{
    // Sample the texture using the red channel (FreeType glyphs are monochrome)
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    // Apply the text color to the glyph
    color = vec4(textColor, 1.0) * sampled;
}

)"