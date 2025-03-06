#version 410

uniform vec4 fs_params[2];
uniform sampler2D tex0_smp;

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 uvCoord;

vec2 uv_iq(vec2 uv, ivec2 texture_size)
{
    vec2 _20 = vec2(texture_size);
    vec2 _27 = floor(fma(uv, _20, vec2(0.5)));
    return (_27 + clamp(fma(uv, _20, -_27) / fwidth(uv * _20), vec2(-0.5), vec2(0.5))) / vec2(texture_size);
}

void main()
{
    vec2 param = ((gl_FragCoord.xy - floor(fs_params[1].xy)) / fs_params[0].xy) * fs_params[0].zw;
    ivec2 param_1 = ivec2(fs_params[0].xy);
    fragColor = texture(tex0_smp, uv_iq(param, param_1));
}

