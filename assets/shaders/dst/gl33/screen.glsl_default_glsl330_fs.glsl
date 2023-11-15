#version 330

uniform vec4 fs_params[2];
uniform sampler2D tex0_smp;

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = texture(tex0_smp, ((gl_FragCoord.xy - fs_params[1].xy) / fs_params[0].xy) * fs_params[0].zw);
}

