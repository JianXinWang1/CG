#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D ssaoInput;

void main() 
{
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
    vec4 result = vec4(0.0,0.0,0.0,0.0);
    int count = 0;
    
    // pcf模糊
    for (int x = -1; x < 1; ++x) 
    {
        for (int y = -1; y < 1; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            if(texture(ssaoInput, TexCoords + offset).r>0){
                result += texture(ssaoInput, TexCoords + offset).rgba;
                count+=1;
            }
        }
    }
    if(count>0){
        FragColor = result /count;
    }
}  