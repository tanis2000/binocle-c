#version 300 es
precision mediump float;
precision highp int;

layout(location = 0) out highp vec4 fragColor;
in highp vec4 color;

void main()
{
    fragColor = color;
}

