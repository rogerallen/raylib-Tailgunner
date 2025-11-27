#version 100

#ifdef GL_ES
precision mediump float;
#endif

// Input vertex attributes
attribute vec3 vertexPosition;
attribute vec3 vertexNormal;
attribute vec2 vertexTexCoord;

// Input uniforms - standard raylib uniforms
uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProjection;

// Output vertex attributes (to fragment shader)
varying vec3 fragPosition;

void main()
{
    // Transform vertex position to world space using the instance model matrix
    vec4 worldPos = matModel * vec4(vertexPosition, 1.0);
    fragPosition = worldPos.xyz;
    
    // Apply view and projection
    gl_Position = matProjection * matView * worldPos;
}
