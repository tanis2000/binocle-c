#version 300 es

uniform vec4 vs_params[12];
layout(location = 0) in vec3 vertexPosition;
out vec4 color;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec2 vertexTexture;

void main()
{
    gl_Position = ((mat4(vs_params[0], vs_params[1], vs_params[2], vs_params[3]) * mat4(vs_params[4], vs_params[5], vs_params[6], vs_params[7])) * mat4(vs_params[8], vs_params[9], vs_params[10], vs_params[11])) * vec4(vertexPosition, 1.0);
    gl_PointSize = 1.0;
    color = vertexColor;
}

