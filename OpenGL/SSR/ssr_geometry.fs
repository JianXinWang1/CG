#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform bool invertedNormals;
uniform sampler2D baseColor;
uniform sampler2D water;

void main()
{    
    gPosition = FragPos;

    gNormal = normalize(Normal);
    // 设置RGB-A掩码
    if(invertedNormals){
        gAlbedo.rgb = texture(water,TexCoords).rgb;
        gAlbedo.a = 0.0f;
    }
    else{
        gAlbedo.rgb = texture(baseColor,TexCoords).rgb;
        gAlbedo.a = 1.0f;
    }
    
}