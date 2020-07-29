#version 330 core

out vec4 fragColor;
in vec3 myColor;

void main()
{
    fragColor = vec4(myColor, 1.0f);
}
