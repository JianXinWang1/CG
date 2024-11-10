#version 330 core
layout (location = 0) out float hiZTexture;

uniform sampler2D baseColor;
uniform sampler2D depthTexture;
uniform vec2 texelSize; 

in vec2 TexCoords;


void main() {
    vec2 uv = TexCoords;
    vec2 offset_lu = vec2(-texelSize.x, texelSize.y);
    vec2 offset_ru = vec2(texelSize.x, texelSize.y);
    vec2 offset_dl = vec2(-texelSize.x, -texelSize.y);
    vec2 offset_rd = vec2(texelSize.x, -texelSize.y);

    // 获取周围四个像素的深度值
    float depth1 = texture(depthTexture, uv + offset_lu).r; 
    float depth2 = texture(depthTexture, uv + offset_ru).r; 
    float depth3 = texture(depthTexture, uv + offset_dl).r; 
    float depth4 = texture(depthTexture, uv + offset_rd).r; 

    // 取最小值
    hiZTexture = max(depth1, max(depth2, max(depth3, depth4)));
}