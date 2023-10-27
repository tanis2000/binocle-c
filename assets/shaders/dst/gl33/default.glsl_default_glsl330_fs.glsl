#version 330

uniform sampler2D tex0_smp;

in vec2 tcoord;
layout(location = 0) out vec4 fragColor;
in vec4 color;

void main()
{
    fragColor = color * texture(tex0_smp, tcoord);
}

