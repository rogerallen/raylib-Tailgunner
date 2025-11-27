#version 330 core

// Input vertex attributes
in vec3 vertexPosition;
in mat4 instanceTransform;

// Input uniforms - standard raylib uniforms
uniform mat4 matView;
uniform mat4 matProjection;
uniform vec4 colDiffuse;

void main()
{
    // Transform vertex position to world space using the instance model matrix
    vec4 worldPos = instanceTransform * vec4(vertexPosition, 1.0);
    
    // Apply view and projection
    gl_Position = matProjection * matView * worldPos;
}
