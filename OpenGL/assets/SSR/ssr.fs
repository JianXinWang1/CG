#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

uniform mat4 projection;
uniform vec3 cameraPosition;
void main()
{      
    // 从gbuffer获取数据
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;

    // 查掩码是否为水面
    float check = texture(gAlbedo, TexCoords).w;

    vec3 camera2Pos = FragPos - cameraPosition;
    
    // 未超出边界
    int board = 0;

    // 步进设置
    const int step = 3999;
    float dt = 0.02; 

    if(check == 0.0f){
        vec3 reflectDir = normalize(reflect(camera2Pos, Normal));
        for(int i = 1; i < step; i++){
            float d = dt * i;
            vec3 sample = FragPos + d * reflectDir;
            vec4 projectPos = projection * vec4(sample, 1.0f);
            vec3 ndcPos = projectPos.xyz / projectPos.w;
            vec2 texPosXY = (ndcPos.xy + 1.0) / 2.0;

            // 如果超出边界，写入水面颜色，记录并结束
            if(texPosXY.x > 1.0 || texPosXY.x < 0.0 || texPosXY.y > 1.0 || texPosXY.y < 0.0){
                FragColor = vec4(Diffuse.rgb, 1.0f);
                board = 1;
                break;
            }

            // 根据深度判断是否被反射
            float depthCheck = sample.z - texture(gPosition, texPosXY.xy).z;
            if(depthCheck < 0 && texture(gAlbedo, texPosXY.xy).w == 1.0 && abs(depthCheck) < 0.06){
                FragColor = vec4(texture(gAlbedo, texPosXY.xy).rgb * Diffuse, 1.0f);
                board = 1;
                break;
            }
        }
    }
    
    // 如果不是水面直接着色
    else{
        FragColor = vec4(Diffuse.rgb,1.0f);
    }
    
    // 如果没有被标记或没有超出屏幕范围则按照原贴图采样颜色
    if(board == 0){
        FragColor = vec4(Diffuse.rgb, 1.0f);
    }
}

    
