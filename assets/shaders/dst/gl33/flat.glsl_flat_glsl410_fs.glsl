#version 410

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec4 color;

void main()
{
    fragColor = color;
}

