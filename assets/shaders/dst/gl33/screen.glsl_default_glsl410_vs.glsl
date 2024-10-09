#version 410

uniform vec4 vs_params[4];
layout(location = 0) in vec3 position;

void main()
{
    gl_Position = mat4(vs_params[0], vs_params[1], vs_params[2], vs_params[3]) * vec4(position, 1.0);
}

