#version 330
in vec3 vertexPosition;
in vec2 vertexTCoord;
in vec4 vertexColor;
in vec3 vertexNormal;

out vec2 tcoord;
out vec4 color;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

void main(void) {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
    tcoord = vertexTCoord;
    color = vertexColor;
    vec3 n = vertexNormal;
    gl_PointSize = 1.0;
}

