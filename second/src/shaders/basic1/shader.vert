#version 330 core

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 color;

out vec4 colorIn;

void main(){
    gl_Position = pos;
    colorIn = color;
}