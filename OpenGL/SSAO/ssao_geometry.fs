#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;

uniform sampler2D modelColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

void main()
{    

    gPosition = FragPos;

    gNormal = normalize(Normal);
    // 对于背景设置为0.95, 模型设置为贴图颜色
    if(texture(modelColor, TexCoords).b>0){
        gAlbedo.rgb = texture(modelColor, TexCoords).rgb;
    }
    else{
        gAlbedo.rgb = vec3(0.95);
    }
}