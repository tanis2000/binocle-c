#ifdef GL_ES
precision highp float;
#endif

uniform vec2 resolution;
uniform sampler2D texture;
uniform vec2 scale;
uniform vec2 viewport;

void main() {

    vec2 uv = (gl_FragCoord.xy - viewport.xy) / resolution.xy * scale;
    gl_FragColor = texture2D( texture, uv );

}

/*
void main() {
  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
*/