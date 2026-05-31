#define N 30


vec3 tfColor = vec3(.5,0,.5);
vec3 tbColor = vec3(.8,0,.7);
vec3 bg = vec3(0.2,0,0.2);

vec2 positions[N];
float speeds[N];
float sizes[N];
float ws[N];

float hash(float n){ return fract(sin(n)*43758.5453); }

void init() {
    for (int i = 0; i < N; i++) {
        float fi = float(i);
        positions[i] = vec2(
            hash(fi) * 2.0 - 1.0,
            hash(fi+2.0) * 2.0 - 1.0
        );
        speeds[i] = 0.2 + hash(fi+0.2)*0.4;
        float size = 0.08 + hash(fi)*0.06;
        sizes[i] = size;
        ws[i] =  0.005 * size;
    }
}

float sdTriangle(vec2 p){
    const float k = 1.7320508;
    p.x = abs(p.x) - 1.0;
    p.y = p.y + 1.0/k;
    vec2 q = (p.x + k*p.y > 0.0) ? vec2(p.x - k*p.y, -k*p.x - p.y)/2.0 : p;
    q.x -= clamp(q.x, -2.0, 0.0);
    return -length(q)*sign(q.y);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5*iResolution.xy) / iResolution.y;
    vec3 col = vec3(0.2,0,0.2); 
    init();

    for (int i=0; i<N; i++){
        vec2 triPos = positions[i];
        triPos.y = mod(triPos.y + iTime*speeds[i], 2.0) - 1.0;

        // skip triangles too far
        if (length(uv - triPos) > sizes[i]*1.5) continue;

        vec2 p = uv - triPos;
        float d = sdTriangle(p / sizes[i]) * sizes[i];

      float thickness = 0.03 * sizes[i]; // border proportional to size
    float fill   = smoothstep(0.0, thickness, -d);
    float border = smoothstep(thickness, 0.0, abs(d) - thickness*0.5);
    float a = clamp(fill + border, 0.0, 1.0);
    vec3 c = mix(tfColor, tbColor, border);
    col = mix(col, c, a);

    }

    fragColor = vec4(col,1.0);
}
