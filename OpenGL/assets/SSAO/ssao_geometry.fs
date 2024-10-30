#version 330 core
layout (location = 0) out vec2 gPosition;
layout (location = 1) out vec2 gNormal;
layout (location = 2) out vec3 gAlbedo;

uniform sampler2D modelColor;
uniform bool invertedNormals;
uniform mat4 projection;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;


void main()
{   
    vec4 projectionXYZW = projection * vec4(FragPos.xyz, 1.0f);
    gPosition = vec2(projectionXYZW.z, projectionXYZW.w);
    vec3 normal = normalize(Normal);
    gNormal = normal.xy;
    // 对于背景设置为0.95, 模型设置为贴图颜色
    if(!invertedNormals){
        gAlbedo.rgb = texture(modelColor, TexCoords).rgb;
    }
    else{
        gAlbedo.rgb = vec3(0.95);
    }
}