#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];

// 数量 采样半径 随机扰动
int kernelSize = 64;
float radius = 0.5;
// 深度是离散的，需要加上保护值
float bias = 0.0;

// 要获取16个随机采样向量需要将-1 1的纹理坐标范围映射出去
const vec2 noiseScale = vec2(800.0/4.0, 600.0/4.0); 

uniform mat4 projection;
uniform mat4 invprojection;
void main()
{
    // 由w值还原projection--->view
    vec2 projectionZW = texture(gPosition, TexCoords).rg;
    vec3 pointPixel = vec3((TexCoords.x * 2 - 1), (TexCoords.y * 2 - 1) , projectionZW.x / projectionZW.y) * projectionZW.y;
    vec4 fragPostem = invprojection * vec4(pointPixel, projectionZW.y);
    vec3 fragPos = fragPostem.xyz;
    vec2 gnormal = texture(gNormal, TexCoords).rg;
    vec3 normal = vec3(gnormal.xy, 1.0 - sqrt(gnormal.x * gnormal.x + gnormal.y * gnormal.y));
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);
    
    // 由随机采样向量生成TBN
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        vec3 samplePos = TBN * samples[i]; // from tangent to view-space

        // view坐标系offset
        samplePos = fragPos + samplePos * radius; 
        
        // projection坐标系offset
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset;
        float offsetW = offset.w;
        offset = offset/offset.w; 

        vec2 offsetTex = offset.xy * 0.5 + 0.5;
        
        // 将zbuffer逆变换到view空间
        projectionZW = texture(gPosition, offsetTex).rg;
        pointPixel = vec3(offsetTex , projectionZW.x / projectionZW.y) * projectionZW.y;
        fragPostem = invprojection * vec4(pointPixel, projectionZW.y);
        vec3 viewFragPos = fragPostem.xyz;

        float sampleDepth = viewFragPos.z;

        // rangeCheck 如果深度相差太大(和半径相对比)了,则减小occlusion
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(samplePos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    FragColor = occlusion;
}
