#version 410

uniform sampler2D tex0_smp;

layout(location = 0) in vec2 tcoord;
layout(location = 0) out vec4 fragColor;
layout(location = 1) in vec4 color;

void main()
{
    fragColor = color * texture(tex0_smp, tcoord);
}

