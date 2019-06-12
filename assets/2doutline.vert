//"out" varyings to our fragment shader
varying vec4 vColor;
varying vec2 vTexCoord;
 
void main() {
    vec4 vertex = gl_ModelViewMatrix * gl_Vertex;
    gl_Position = gl_ProjectionMatrix * vertex;
	vColor = gl_Color;
	vTexCoord = vec2(gl_TextureMatrix[0] * gl_MultiTexCoord0);
}