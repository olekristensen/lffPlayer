#version 120

uniform sampler2D tex;

varying vec2 texCoordVarying;

void main() {
    vec3 rawColor = texture2D(tex, texCoordVarying).rgb;
    
    float texCoordYWithoutText = (texCoordVarying.y * (1100.0/1080.0));
    
    if(texCoordVarying.x > 0.5 && texCoordYWithoutText > 0.5 && texCoordYWithoutText <= 1.0){
        vec3 rawColorLow =rawColor * 0.93;
        rawColorLow.g = rawColorLow.g * 0.9;
        rawColorLow.r = rawColorLow.r * 0.9;
        float mixFactor = (rawColor.r + rawColor.g + rawColor.b) / 3.0;
        mixFactor = pow(mixFactor,0.09);
        rawColor = (mixFactor * rawColor) + ((1.0-mixFactor) * rawColorLow);
    }
    
    gl_FragColor = vec4(rawColor, 1.0);
    
}