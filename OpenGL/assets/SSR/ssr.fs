#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gDepth;
uniform sampler2D hizDepth1;
uniform sampler2D hizDepth2;

uniform mat4 projection;
uniform vec3 cameraPosition;


vec3 viewPoint2ndcTex(vec3 viewPoint){
    vec4 projectPos = projection * vec4(viewPoint, 1.0f);
    vec3 ndcPos = projectPos.xyz / projectPos.w;
    vec3 texPosXYZ = (ndcPos.xyz + 1.0f) / 2.0f;
    return texPosXYZ;
}

float computeW(vec3 viewPoint){
    vec4 projectPos = projection * vec4(viewPoint, 1.0f);
    return projectPos.w;
}

void main()
{   
    // 从gbuffer获取数据
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;
    // float debugDepth = texture(gDepth, TexCoords).r;
    

    int mipScale[3];
    mipScale[0] = 1;
    mipScale[1] = 2;
    mipScale[2] = 4;

    // 查掩码是否为水面
    float check = texture(gAlbedo, TexCoords).w;

    vec3 camera2Pos = FragPos - cameraPosition;
    
    // 未超出边界
    int board = 0; 
    
    // 是水面进行SSR
    if(check == 0.0f){
        vec3 reflectDir = normalize(reflect(camera2Pos, Normal));
        
        // 设定屏幕起点与终点
        int stepEnd = 99999;
        vec3 stepEndPos = FragPos + stepEnd * reflectDir;
        vec3 stepEndPosTem = viewPoint2ndcTex(stepEndPos);
        vec3 start = viewPoint2ndcTex(FragPos);
        vec3 end = stepEndPosTem;
        vec2 texDir = normalize(vec2(end.xy - start.xy));

        // DDA
        float dx = end.x - start.x;
        float dy = end.y - start.y;
        float k = dy / dx;  //斜率
        float e = abs(k) >= 1 ?abs(dy):abs(dx);
        float xadd = dx / e;
        float yadd = dy / e;
        float x = start.x;
        float y = start.y;
        int mipLevel = 0;
        int count = 0;

        // mip trace
        while(true){
            if(count > 9999){
                break;
            }

            // mip trace 屏幕空间步进
            float curX = x + 1.0f/800 * xadd * mipScale[mipLevel];
            float curY = y + 1.0f/600 * yadd * mipScale[mipLevel];

            // 超出屏幕未SSR直接写入水面颜色
            if(curX < 0 || curX > 1 || curY < 0 || curY > 1){
                FragColor = vec4(Diffuse.rgb, 1.0f);
                return;
            }
            float alpha = 0;
            float beta = 0;
            if(abs(k) <= 1){
                alpha = 1.0f - abs(curX - start.x) / abs(start.x - end.x);
                beta = 1.0f - alpha;
            }
            else{
                alpha = 1.0f - abs(curY - start.y) / abs(start.y - end.y);
                beta = 1.0f - alpha;
            }

            float curZ = alpha * start.z + beta * end.z;

            vec2 texPosXY = vec2(curX, curY);
            vec4 proPoint;
            if (mipLevel == 0){
                proPoint = projection * vec4(vec3(texture(gPosition, texPosXY.xy).xy, texture(gDepth, texPosXY.xy).r), 1.0f);
            }
            else if (mipLevel == 1){
                proPoint = projection * vec4(vec3(texture(gPosition, texPosXY.xy).xy, texture(hizDepth1, texPosXY.xy).r), 1.0f);
            }
            else{
                proPoint = projection * vec4(vec3(texture(gPosition, texPosXY.xy).xy, texture(hizDepth2, texPosXY.xy).r), 1.0f);
            }
            float proZ = (proPoint/proPoint.w).z / 2.0f + 0.5;
            float depthCheck = curZ - proZ;

            //未命中则miplevel++
            if(depthCheck > 0 && mipLevel < 2){
                mipLevel++;
            }
            
            // 命中条件
            if(depthCheck < 0 && abs(depthCheck) < 0.00020 && texture(gAlbedo, vec2(curX, curY)).w == 1.0){
                if(mipLevel == 0){
                    FragColor = vec4(texture(gAlbedo, vec2(curX, curY)).rgb * Diffuse, 1.0f);
                    return;
                }
                else{
                    mipLevel--;
                }
            }

            x = curX;
            y = curY;  
            count ++;
        }
    }
    
    // 如果不是水面直接着色
    else{
        FragColor = vec4(Diffuse.rgb,1.0f);
    }  
}

    
