#pragma sokol @vs vs

in vec3 vertexPosition;
in vec4 vertexColor;
in vec2 vertexTexture;

uniform vs_params {
    mat4 projectionMatrix;
    mat4 viewMatrix;
    mat4 modelMatrix;
};

out vec4 color;

void main(void) {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
   //gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
    gl_PointSize = 1.0;
    color = vertexColor;
}

#pragma sokol @end

#pragma sokol @fs fs

in vec4 color;
out vec4 fragColor;
void main(void)
{
    fragColor = color;
    //fragColor = vec4(1.0, 1.0, 1.0, 1.0);
}

#pragma sokol @end

#pragma sokol @program flat vs fs