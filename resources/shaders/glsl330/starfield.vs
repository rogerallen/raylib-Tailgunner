#version 330 core

// Input vertex attributes
in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;

// Input uniforms - standard raylib uniforms
uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProjection;

void main()
{
    // Transform vertex position to world space using the instance model matrix
    vec4 worldPos = matModel * vec4(vertexPosition, 1.0);
    
    // Apply view and projection
    gl_Position = matProjection * matView * worldPos;
}
