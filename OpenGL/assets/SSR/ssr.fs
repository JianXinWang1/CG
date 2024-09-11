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
    const int step = 3999;
    float dt = 0.02;    
    // 从gbuffer获取数据
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    float judge = texture(gAlbedo, TexCoords).w;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;

    vec3 camera2Pos = FragPos - cameraPosition;
    
    // 未超出边界
    int res = 0;
    // rgb-a掩码

    if(judge == 0.0f){

        vec3 reflectDir = normalize(reflect(camera2Pos, Normal));
        for(int i=1;i<step;i++){
            float d= dt * i;
            vec3 newPos = FragPos+d*reflectDir;
            vec4 pnewPos = projection * vec4(newPos,1.0f);
            vec3 pnewPos3 = pnewPos.xyz/pnewPos.w;
            vec2 pos2Tex = (pnewPos3.xy+1.0)/2.0;
            if(pos2Tex.x>1.0 || pos2Tex.x<0.0 ||pos2Tex.y>1.0||pos2Tex.y<0.0){
                FragColor = vec4(Diffuse.rgb,1.0f);
                res = 1;
                break;
            }
            float depthsub = newPos.z - texture(gPosition,pos2Tex.xy).z;
            if(depthsub<0 && texture(gAlbedo,pos2Tex.xy).w ==1.0&&abs(depthsub)<0.06){
                FragColor = vec4(texture(gAlbedo,pos2Tex.xy).rgb * Diffuse,1.0f);
                res = 1;
                break;
            }
        }
    }
    else{
        FragColor = vec4(Diffuse.rgb,1.0f);
    }
    if(res==0){
        FragColor = vec4(Diffuse.rgb,1.0f);
    }
    

}

    
