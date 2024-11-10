#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out float gDepth;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform bool invertedNormals;
uniform sampler2D baseColor;
uniform sampler2D water;

void main()
{   float n = -0.1, f = -50.0;
    gPosition = FragPos;
    float linearZ =  (FragPos.z - n) / (f - n);
    gDepth = FragPos.z;
    gNormal = normalize(Normal);
    // 设置RGB-A掩码
    if(invertedNormals){
        gAlbedo.rgb = texture(water, TexCoords).rgb;
        gAlbedo.a = 0.0f;
    }
    else{
        gAlbedo.rgb = texture(baseColor, TexCoords).rgb;
        gAlbedo.a = 1.0f;
    }
    
}