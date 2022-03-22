precision highp float;

uniform vec2 resolution;
uniform sampler2D tex0;
uniform vec2 scale;
uniform vec2 viewport;

void main() {

    vec2 uv = (gl_FragCoord.xy - viewport.xy) / resolution.xy * scale;
    gl_FragColor = texture2D( tex0, uv );

}

/*
void main() {
  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
*/