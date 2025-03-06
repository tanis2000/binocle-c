#version 300 es
precision mediump float;
precision highp int;

uniform highp vec4 fs_params[2];
uniform highp sampler2D tex0_smp;

layout(location = 0) out highp vec4 fragColor;
in highp vec2 uvCoord;

highp vec2 uv_iq(highp vec2 uv, ivec2 texture_size)
{
    highp vec2 _20 = vec2(texture_size);
    highp vec2 _27 = floor(uv * _20 + vec2(0.5));
    return (_27 + clamp((uv * _20 + (-_27)) / fwidth(uv * _20), vec2(-0.5), vec2(0.5))) / vec2(texture_size);
}

void main()
{
    highp vec2 param = ((gl_FragCoord.xy - floor(fs_params[1].xy)) / fs_params[0].xy) * fs_params[0].zw;
    ivec2 param_1 = ivec2(fs_params[0].xy);
    fragColor = texture(tex0_smp, uv_iq(param, param_1));
}

