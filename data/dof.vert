varying vec2 vUv;
attribute vec3 position;
attribute vec2 uv;


void main() {
  vUv = uv;
  gl_Position = vec4( position, 1.0 );
}
