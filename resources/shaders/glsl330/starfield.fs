#version 330 core

// Uniform diffuse color (set from the application)
uniform vec4 colDiffuse;

// Output color
out vec4 fragColor;

void main()
{
    // Use application-provided color for stars
    fragColor = colDiffuse;
}
