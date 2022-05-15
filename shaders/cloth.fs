#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

struct PointLight {
    vec3 position;
    vec3 color;
    float quadratic_attenuation;
};

struct Thread {
    vec3 albedo;  // Colored albedo coefﬁcient
    float k_d;  // Isotropic scattering coefﬁcient
    float gamma_s;  // Surface reflectance Gaussian width
    float gamma_v;  // Volume scattering Gaussian width
    float a;  //  Area coverage ratio
    int tangent_num;
    vec3 tangents[10];  // max 10
    float tangent_weights[10];
};

uniform mat4 model;
uniform vec3 viewPos;
#define NR_POINT_LIGHTS 2
uniform PointLight pointLights[NR_POINT_LIGHTS];

uniform Thread threads[2];

float F0  = 0.04;


vec3 fresnel(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float gaussian(float theta, float mean, float std) {
    return exp(-((theta - mean) * (theta - mean)) / (2 * std * std));
}

vec3 get_local_normal(vec3 t, vec3 omega) {
    vec3 nt = normalize(t);
    return normalize(omega - dot(nt, omega) * nt);
}

vec3 surface_reflection(Thread thread, vec3 light, vec3 view, vec3 t, vec3 normal) {
    vec3 normal_i = get_local_normal(t, light);
    vec3 normal_r = get_local_normal(t, view);

    float cos_theta_i = dot(light, normal_i);
    float cos_theta_r = dot(view, normal_r);
    float theta_half = (acos(clamp(cos_theta_i, -1, 1)) + acos(clamp(cos_theta_r, -1, 1))) / 2;
    
    float phi_i = acos(dot(normal_i, normal));
    float phi_r = acos(dot(normal_r, normal));
    float phi_d = phi_i - phi_r;

    vec3 f = fresnel(cos(theta_half) * cos(phi_d / 2), vec3(F0, F0, F0) * thread.albedo);

    return f * cos(phi_d / 2) * gaussian(theta_half, 0, thread.gamma_s);
}

vec3 volume_scatter(Thread thread, vec3 lightDir, vec3 view, vec3 t, vec3 normal) {
    vec3 normal_i = get_local_normal(t, lightDir);
    vec3 normal_r = get_local_normal(t, view);

    float cos_theta_i = clamp(dot(lightDir, normal_i), -1, 1);
    float cos_theta_r = clamp(dot(view, normal_r), -1, 1);
    float theta_half = (acos(clamp(cos_theta_i, -1, 1)) + acos(clamp(cos_theta_r, -1, 1))) / 2;

    vec3 f_i = fresnel(dot(lightDir, normal), vec3(F0, F0, F0) * thread.albedo);
    vec3 f_r = fresnel(dot(view, normal), vec3(F0, F0, F0) * thread.albedo);

    float cof = (1 - thread.k_d) * gaussian(theta_half, 0, thread.gamma_v) + thread.k_d;
    cof /= (cos_theta_i + cos_theta_r + 0.001);

    return f_i * f_r * cof * thread.albedo;
}

float masking(vec3 lightDir, vec3 view, vec3 t, vec3 normal) {
    vec3 normal_i = get_local_normal(t, lightDir);
    vec3 normal_r = get_local_normal(t, view);
    float phi_i = acos(clamp(dot(normal_i, normal), -1, 1));
    float phi_r = acos(clamp(dot(normal_r, normal), -1, 1));
    float phi_d = phi_i - phi_r;

    float mask_i = max(cos(phi_i), 0);
    float mask_r = max(cos(phi_r), 0);
    float u = gaussian(phi_d, 0, 3.14 / 9);

    return (1 - u) * mask_i * mask_r + u * min(mask_i, mask_r);
}

float projection(vec3 lightDir, vec3 view, vec3 t, vec3 normal) {
    vec3 c = normalize(cross(t, normal));
    float cos_i = dot(lightDir - dot(lightDir, c) * c, normal);
    float cos_r = dot(view - dot(view, c) * c, normal);
    float psi_i = acos(clamp(cos_i, -1, 1));
    float psi_r = acos(clamp(cos_r, -1, 1));
    float psi_d = psi_i - psi_r;

    float pro_i = max(cos(psi_i), 0);
    float pro_r = max(cos(psi_r), 0);
    float u = gaussian(psi_d, 0, 3.14 / 9);

    return (1 - u) * pro_i * pro_r + u * min(pro_i, pro_r);
}

vec3 renderThread(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);

    vec3 Lr_1 = vec3(0.0, 0.0, 0.0);
    vec3 Lr_2 = vec3(0.0, 0.0, 0.0);
    float Q = (1 - threads[0].a - threads[1].a) * dot(viewDir, normal);

    for (int i = 0; i < 2; i++) {  // for two direction's threads
        for (int j = 0; j < threads[i].tangent_num; ++j) {
            vec3 tangent = vec3(model * vec4(threads[i].tangents[j], 1.0));

            vec3 normal_i = get_local_normal(tangent, lightDir);
            vec3 normal_r = get_local_normal(tangent, viewDir);
            float cos_theta_i = dot(lightDir, normal_i);
            float cos_theta_r = dot(viewDir, normal_r);
            float cos_theta_d = cos((acos(clamp(cos_theta_i, -1, 1)) - acos(clamp(cos_theta_r, -1, 1))) / 2);

            vec3 f_rs = surface_reflection(threads[i], lightDir, viewDir, tangent, normal);
            vec3 f_rv = volume_scatter(threads[i], lightDir, viewDir, tangent, normal);
            vec3 f_s = (f_rs + f_rv) / (cos_theta_d * cos_theta_d + 0.0001);

            float m_t = masking(lightDir, viewDir, tangent, normal);
            float p_t = projection(lightDir, viewDir, tangent, normal);

            float dist = length(light.position - fragPos);
            float attenuation = 1.0 / (light.quadratic_attenuation * (dist * dist));    
            vec3 L_i = light.color * attenuation * max(dot(normal, lightDir), 0.0);

            vec3 Lr_j = threads[i].a * L_i * f_s * m_t * p_t * threads[i].tangent_weights[j];
            
            if (i == 0) Lr_1 += Lr_j;
            if (i == 1) Lr_2 += Lr_j;
            Q += p_t * threads[i].a * threads[i].tangent_weights[j];
        }
    }
    
    return (Lr_1 + Lr_2) / (Q + 0.0001);
}

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    vec3 result = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += renderThread(pointLights[i], norm, FragPos, viewDir);      
    
    FragColor = vec4(result * 2, 1.0);
} 
