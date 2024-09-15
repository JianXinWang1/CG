#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 lightSpacePos;
} fs_in;

uniform sampler2D modelTexture;
uniform sampler2D depthTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;

// 平均遮挡物的filter,用于后续相似三角形映射计算pcss
float blockDepth(vec3 lightPixel,vec2 texSize){
    int count = 0;
    float depthBlock = 0.0;
    for(int i = -3;i<=3;i++){
        for(int j =-3;j<=3;j++){
            if(lightPixel.x+i*texSize.x < 0 || lightPixel.x+i*texSize.x>1 || lightPixel.y+j*texSize.y<0 ||lightPixel.y+j*texSize.y>1){
                continue;
            }
            float depthValue = texture(depthTexture, vec2(lightPixel.x+i*texSize.x,lightPixel.y+j*texSize.y)).r;
            float currentDepth = lightPixel.z;
            if(currentDepth-0.01 > depthValue){
                depthBlock+=depthValue;
                count+=1;
            }
        }
    }
    depthBlock = depthBlock/count;
    return depthBlock;
}

void main()
{   
    float size = 1;
    vec2 texSize = 1.0/textureSize(depthTexture,0);

    vec3 lightPixel = fs_in.lightSpacePos.xyz;
    lightPixel.xyz = lightPixel.xyz*0.5+0.5;

    float shadow = 0.0;

    if(lightPixel.x >= 0 && lightPixel.x<=1 && lightPixel.y>=0 &&lightPixel.y<=1 && lightPixel.z-0.01>
    texture(depthTexture, vec2(lightPixel.x,lightPixel.y)).r){
        
        float blockD = blockDepth(lightPixel,texSize);
        float posD = lightPixel.z;
        float div = size*(posD-blockD)/blockD;
        int culSize = clamp(int(div),0,5);

        // PCF filter
        if(culSize!=0&&culSize!=1){
            for(int i = -culSize;i<=culSize;i++){
                for(int j =-culSize;j<=culSize;j++){
                    if(lightPixel.x+i*texSize.x < 0 || lightPixel.x+i*texSize.x>1 || lightPixel.y+j*texSize.y<0 ||lightPixel.y+j*texSize.y>1){
                        continue;
                    }
                    float depthValue = texture(depthTexture, vec2(lightPixel.x+i*texSize.x,lightPixel.y+j*texSize.y)).r;
                    float currentDepth = lightPixel.z;
                    if(currentDepth-0.01 > depthValue){
                        shadow+=1.0;
                    }
                }
            }
            shadow = shadow/((culSize*2+1)*(culSize*2+1));
        }
        else{
            shadow = 1.0;
        }
    }
    
    vec3 color = texture(modelTexture, fs_in.TexCoords).rgb;
    // ambient
    vec3 ambient = 0.1 * color;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 normal = normalize(fs_in.Normal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
   
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    
   
    vec3 specular = vec3(0.3) * spec; // assuming bright white light color
  
    FragColor = vec4(ambient + (1.0-shadow)*(diffuse + specular), 1.0);
   
}
