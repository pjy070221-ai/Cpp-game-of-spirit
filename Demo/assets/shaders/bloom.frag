// ═══════════════════════════════════════════════
// Bloom 发光着色器
// 对亮度超过阈值的像素产生辉光扩散
// uniform: threshold (0.0~1.0), intensity (0.0~2.0)
// ═══════════════════════════════════════════════

#version 330 core

uniform sampler2D texture;      // 场景渲染纹理
uniform vec2  texSize;          // 纹理尺寸（用于计算像素偏移）
uniform float threshold = 0.6;  // 亮度阈值
uniform float intensity = 1.0;  // 辉光强度

in vec2 texCoord;
out vec4 fragColor;

void main()
{
    vec4 color = texture2D(texture, texCoord);

    // 计算亮度
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));

    // 超过阈值的部分产生辉光
    float glow = max(0.0, brightness - threshold);

    // 水平 + 垂直模糊近似（简单的 9 像素采样）
    vec2 pixelSize = 1.0 / texSize;
    vec3 blurred = vec3(0.0);

    float weights[9] = float[](
        0.0625, 0.125, 0.0625,
        0.125,  0.25,  0.125,
        0.0625, 0.125, 0.0625
    );

    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            vec2 offset = vec2(float(x), float(y)) * pixelSize * 3.0;
            vec3 sample = texture2D(texture, texCoord + offset).rgb;
            blurred += sample * weights[(x + 1) * 3 + (y + 1)];
        }
    }

    // 合成：原图 + 模糊辉光
    vec3 result = color.rgb + blurred * glow * intensity;

    fragColor = vec4(result, color.a);
}
