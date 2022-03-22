#version 330

uniform vec2 resolution;
uniform sampler2D tex0;
uniform vec2 scale;
uniform vec2 viewport;
out vec4 fragColor;

void main() {

    vec2 uv = (gl_FragCoord.xy - viewport.xy) / resolution.xy * scale;
    fragColor = texture( tex0, uv );

}

/*
void main() {
  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
*/