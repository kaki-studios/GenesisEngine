$input v_normal, v_pos

// uniform vec4 u_lightDir; 
//has to be vec4, uniforms can't be vec3
uniform vec4 u_lightPos;
uniform vec4 u_baseCol;
uniform vec4 u_lightCol;

#include "bgfx_shader.sh"
#include "shaderlib.sh"

void main() {
  vec3 difference = u_lightPos.xyz - v_pos;
  float distance = length(difference);
  // float distance = 10.0;
  vec3 L = normalize(difference);

  
  float attenuation = 1.0 / (1.0 + (0.0001 * distance * distance));
  // float attenuation = 1.0;
  //map [0,1] to [0.2,1] for ambient lighting
  float ndotl = max(dot(normalize(v_normal), L), 0.2);
  float diffuse = min(ndotl * attenuation, 1.0);
  vec3 color = u_lightCol.rgb * u_baseCol.rgb * diffuse;
  
  gl_FragColor = vec4(color, 1.0);
  // gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
  
}
