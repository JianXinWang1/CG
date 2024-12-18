#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

// declare an interface block; see 'Advanced GLSL' for what these are.
out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 lightSpacePos;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightMat4;

void main()
{
    vs_out.lightSpacePos = lightMat4*model*vec4(aPos,1.0f);
    vs_out.FragPos = vec3(model * vec4(aPos,1.0f));
    vs_out.Normal = mat3(transpose(inverse(model)))*aNormal;
    vs_out.TexCoords = aTexCoords;
    gl_Position = projection * view *model* vec4(aPos, 1.0);
}