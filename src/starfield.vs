#version 330

// Input vertex attributes
in vec3 vertexPosition;
in mat4 instanceTransform;

// Input uniform
uniform mat4 mvp;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;

void main()
{
    vec4 newPosition = vec4(vertexPosition, 1.0);
    vec4 worldPosition = instanceTransform * newPosition;

    gl_Position = mvp * worldPosition;
    fragPosition = worldPosition.xyz;
}
