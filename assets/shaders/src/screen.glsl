#pragma sokol @vs vs
in vec3 position;
uniform vs_params {
    mat4 transform;
};

void main() {
    gl_Position = transform * vec4( position, 1.0 );
}
#pragma sokol @end

#pragma sokol @fs fs
uniform fs_params {
    vec2 resolution;
    vec2 scale;
    vec2 viewport;
};
uniform texture2D tex0;
uniform sampler smp;
out vec4 fragColor;

void main() {

    vec2 uv = (gl_FragCoord.xy - viewport.xy) / resolution.xy * scale;
    fragColor = texture(sampler2D(tex0, smp), uv );

}

/*
void main() {
  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
*/
#pragma sokol @end

#pragma sokol @program default vs fs
