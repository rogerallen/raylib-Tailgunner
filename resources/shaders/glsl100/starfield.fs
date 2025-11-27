#version 100

#ifdef GL_ES
precision mediump float;
#endif

// Uniform diffuse color (set from the application)
uniform vec4 colDiffuse;

void main()
{
    // Use application-provided color for stars
    gl_FragColor = colDiffuse;
}
