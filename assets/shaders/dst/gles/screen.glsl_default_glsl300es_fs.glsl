#version 300 es
precision mediump float;
precision highp int;

uniform highp vec4 fs_params[2];
uniform highp sampler2D tex0_smp;

layout(location = 0) out highp vec4 fragColor;

void main()
{
    fragColor = texture(tex0_smp, ((gl_FragCoord.xy - fs_params[1].xy) / fs_params[0].xy) * fs_params[0].zw);
}

