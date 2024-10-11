#pragma sokol @vs vs
in vec3 vertexPosition;
in vec4 vertexColor;
in vec2 vertexTCoord;
in vec3 vertexNormal;

out vec2 tcoord;
out vec4 color;

uniform vs_params {
    mat4 projectionMatrix;
    mat4 viewMatrix;
    mat4 modelMatrix;
};

void main(void) {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
    tcoord = vertexTCoord;
    color = vertexColor;
    vec3 n = vertexNormal;
    gl_PointSize = 1.0;
}
#pragma sokol @end

#pragma sokol @fs fs

uniform texture2D tex0;
uniform sampler smp;
in vec2 tcoord;
in vec4 color;
out vec4 fragColor;

void main(void) {
    vec4 texcolor = texture(sampler2D(tex0, smp), tcoord);
    fragColor = color * texcolor;
}
#pragma sokol @end

#pragma sokol @program default vs fs
