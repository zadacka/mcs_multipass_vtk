// PASSTHROUGH
//////////////

#version 440
layout(location = 0) in vec3 vertex;
uniform mat4 modelview;
uniform mat4 projection;

void main(){
     gl_Position = vec4(vertex, 1); //projection * modelview * vec4(vertex, 1.0);
}
