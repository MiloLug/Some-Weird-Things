#version 330 core

layout(location = 0) in vec4 pos;

uniform mat4 transform;

void main(){
    gl_Position = pos * transform;
}