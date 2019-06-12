//uniform mat4 u_projTrans;

//attribute vec4 a_position;
//attribute vec2 a_texCoord0;
//attribute vec4 a_color;

varying vec4 v_color;
varying vec2 v_texCoord;

uniform vec2 u_viewportInverse;

void main() {
    vec4 vertex = gl_ModelViewMatrix * gl_Vertex;
    gl_Position = gl_ProjectionMatrix * vertex;
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
    gl_FrontColor = gl_Color;

    //gl_Position = u_projTrans * a_position;
    v_texCoord = vec2(gl_TexCoord[0]);
    v_color = gl_FrontColor;
}

