attribute vec3 position; 


void main() { 
  gl_Position = vec4( position, 1.0 ); 
}

/*
void main()
{	
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = ftransform();		
}
*/