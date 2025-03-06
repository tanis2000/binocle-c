#pragma sokol @vs vs
in vec3 position;
uniform vs_params {
    mat4 transform;
};
out vec2 uvCoord;

void main() {
    gl_Position = transform * vec4( position, 1.0 );
    uvCoord = (position.xy + vec2(1,1))/2.0;
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
in vec2 uvCoord;

vec2 uv_iq( vec2 uv, ivec2 texture_size ) {
    vec2 pixel = uv * texture_size;

    vec2 seam = floor(pixel + 0.5);
    vec2 dudv = fwidth(pixel);
    pixel = seam + clamp( (pixel - seam) / dudv, -0.5, 0.5);

    return pixel / texture_size;
}

void main() {

    vec2 uv = (gl_FragCoord.xy - floor(viewport.xy)) / resolution.xy * scale;
    vec2 pixelPerfectUV = uv_iq(uv, ivec2(resolution.xy));
    fragColor = texture(sampler2D(tex0, smp), pixelPerfectUV);
}

#pragma sokol @end

#pragma sokol @program default vs fs
