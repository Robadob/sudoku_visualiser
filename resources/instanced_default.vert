#version 430

uniform mat3 _normalMat;
uniform mat4 _projectionMat;
uniform mat4 _viewMat;
uniform mat4 _modelMat;

in vec3 _vertex;
in vec3 _normal;
in vec2 _texCoords;

uniform samplerBuffer x_pos;
uniform samplerBuffer y_pos;
uniform samplerBuffer z_pos;

out vec3 eyeVertex;
out vec3 eyeNormal;
out vec2 texCoords;

void main()
{
  // Apply model matrix to raw vertex
  vec4 vert = _modelMat * vec4(_vertex,1.0f);
  //Grab model offset from texture array
  vec3 loc_data = vec3(texelFetch(x_pos, gl_InstanceID).x, texelFetch(y_pos, gl_InstanceID).x, texelFetch(z_pos, gl_InstanceID).x);
  // Apply loc_data translation to vert
  vert.xyz += loc_data;
  // Calculate eye vertex
  eyeVertex = (_viewMat * vert).rgb;
  // Calculate frag vertex
  gl_Position = _projectionMat * vec4(eyeVertex, 1.0f);
  
  
  // Calc eye normal
  // From testing with deer model i'm not 100% happy that phong (full bright) works properly, bottom of each foot had different normal (before _normalMat transform)
  eyeNormal = normalize(_normalMat * normalize(_normal));
  // Calc tex coords
  texCoords = _texCoords;
}