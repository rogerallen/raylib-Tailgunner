#version 330 core

// Output fragment color
out vec4 fragColor;

void main()
{
    // VGREEN color: 0x30 0xf0 0xe0 0xff = (48, 240, 224, 255) normalized to (0.188, 0.941, 0.878, 1.0)
    fragColor = vec4(0.188, 0.941, 0.878, 1.0);
}
