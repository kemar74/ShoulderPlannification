#version 430

out vec4 fragColor;

struct PositionalLight {
    vec4 ambiant;
    vec4 diffuse;
    vec4 specular;
    vec3 position;
};

struct Material {
    vec4 ambiant;
    vec4 diffuse;
    vec4 specular;
    float shininness;
};

uniform vec4 globalAmbiant;
uniform PositionalLight light;
uniform Material material;

uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;

in vec4 varyingColor;
in vec3 varyingHalfH;

in vec3 varyingNormal;
in vec3 varyingLightDir;
in vec3 varyingVertPos;
in vec3 vertEyeSpacePos;
in vec3 initialVertPos;

void main(void)
{
    fragColor = vec4(1.0, 1.0, 1.0, 1.0) * varyingColor;

    vec3 N = normalize(varyingNormal);
    vec3 L = normalize(varyingLightDir);
    vec3 V = normalize(-varyingVertPos);
    vec3 R = reflect(-L, N);
    vec3 H = normalize(varyingHalfH);
    float cosTheta = dot(L, N);
    float cosPhi = dot(H, N);

    vec4 ambiant = ((globalAmbiant * material.ambiant) + (light.ambiant * material.ambiant));
    vec4 diffuse = light.diffuse * material.diffuse * max(cosTheta, 0.0);
    vec4 specular = light.specular * material.specular * pow(max(cosPhi, 0.0), material.shininness * 3.0);

    fragColor = vec4(ambiant + diffuse + specular);

}
