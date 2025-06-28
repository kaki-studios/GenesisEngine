$input v_normal

uniform vec4 u_lightDir; //has to be vec4, uniforms can't be vec3
uniform vec4 u_baseCol;

#include "bgfx_shader.sh"
#include "shaderlib.sh"

void main() {
  //idk if normalization is necessary
  vec3 lightDir = normalize(u_lightDir.xyz);
  //0.1 for ambient lighting
  float diffuse = max(dot(normalize(v_normal), lightDir), 0.0)+0.1;
  vec3 color = u_baseCol.rgb * diffuse;
  
  gl_FragColor = vec4(color, 1.0);
  
}
