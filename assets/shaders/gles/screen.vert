#version 300 es
precision mediump float;
precision mediump int;

in vec3 position;
uniform mat4 transform;

void main() {
    gl_Position = transform * vec4( position, 1.0 );
}