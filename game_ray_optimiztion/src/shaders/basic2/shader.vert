#version 330 core

layout(location = 0) in vec4 pos;
out vec4 ShadowCoord;


void main(){
    gl_Position = pos;
    ShadowCoord = gl_Position;
}