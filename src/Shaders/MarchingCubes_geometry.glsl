/**** Geometry Shader Marching Cubes
* Copyright Cyril Crassin, Junuary 2007.
* This code is partially based on the example of
* Paul Bourke \”Polygonising a scalar field\” located at :
* http://local.wasp.uwa.edu.au/~pbourke/geometry/polygonise/
****/

//GLSL version 1.20
#version 430
//New G80 extensions
#extension GL_EXT_geometry_shader4 : enable
#extension GL_EXT_gpu_shader4 : enable

layout ( points ) in;
layout ( triangle_strip, max_vertices = 16 ) out;

//Volume data field texture
uniform sampler3D dataFieldTex;
//Edge table texture
uniform isampler2D edgeTableTex;
//Triangles table texture
uniform isampler2D triTableTex;

//Global iso level
uniform float isolevel;
//Marching cubes vertices decal
uniform vec3 vertDecals[8];

uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;

//Vertices position for fragment shader
in vec3 initialVertPos[];
in vec3 realNormal[];

out vec3 ginitialVertPos;
out vec3 grealNormal;

//Get vertex i position within current marching cube
vec3 cubePos(int i){
    return vec4(vec4(initialVertPos[0].xyz + vertDecals[i], 1.0)).xyz;
}

//Get vertex i value within current marching cube
float cubeVal(int i){
    vec3 pos = cubePos(i);
    vec3 texSize = textureSize(dataFieldTex, 0);
    if (pos.x == 0 || pos.y == 0 || pos.z == 0) return -1;
    if (pos.x >= texSize.x || pos.y >= texSize.y || pos.z >= texSize.z) return -1;
    vec4 val = texture(dataFieldTex, cubePos(i)/texSize);
    return val.a;
}

//Get triangle table value
int triTableValue(int i, int j){
    return texelFetch2D(triTableTex, ivec2(j, i), 0).a;
}

//Compute interpolated vertex along an edge
vec3 vertexInterp(float isolevel, vec3 v0, float l0, vec3 v1, float l1){
    return mix(v0, v1, (isolevel-l0)/(l1-l0));
}

//Geometry Shader entry point
void main(void) {
    vec4 position = vec4(initialVertPos[0], 1.0);
    int cubeindex=0;

    float cubeVal0 = cubeVal(0);
    float cubeVal1 = cubeVal(1);
    float cubeVal2 = cubeVal(2);
    float cubeVal3 = cubeVal(3);
    float cubeVal4 = cubeVal(4);
    float cubeVal5 = cubeVal(5);
    float cubeVal6 = cubeVal(6);
    float cubeVal7 = cubeVal(7);

    //Determine the index into the edge table which
    //tells us which vertices are inside of the surface
    cubeindex = int(cubeVal0 < isolevel);
    cubeindex += int(cubeVal1 < isolevel)*2;
    cubeindex += int(cubeVal2 < isolevel)*4;
    cubeindex += int(cubeVal3 < isolevel)*8;
    cubeindex += int(cubeVal4 < isolevel)*16;
    cubeindex += int(cubeVal5 < isolevel)*32;
    cubeindex += int(cubeVal6 < isolevel)*64;
    cubeindex += int(cubeVal7 < isolevel)*128;

    //Cube is entirely in/out of the surface
    if (cubeindex ==0 || cubeindex == 255)
        return;

    vec3 vertlist[12];

    //Find the vertices where the surface intersects the cube
    vertlist[0] = vertexInterp(isolevel, cubePos(0), cubeVal0, cubePos(1), cubeVal1);
    vertlist[1] = vertexInterp(isolevel, cubePos(1), cubeVal1, cubePos(2), cubeVal2);
    vertlist[2] = vertexInterp(isolevel, cubePos(2), cubeVal2, cubePos(3), cubeVal3);
    vertlist[3] = vertexInterp(isolevel, cubePos(3), cubeVal3, cubePos(0), cubeVal0);
    vertlist[4] = vertexInterp(isolevel, cubePos(4), cubeVal4, cubePos(5), cubeVal5);
    vertlist[5] = vertexInterp(isolevel, cubePos(5), cubeVal5, cubePos(6), cubeVal6);
    vertlist[6] = vertexInterp(isolevel, cubePos(6), cubeVal6, cubePos(7), cubeVal7);
    vertlist[7] = vertexInterp(isolevel, cubePos(7), cubeVal7, cubePos(4), cubeVal4);
    vertlist[8] = vertexInterp(isolevel, cubePos(0), cubeVal0, cubePos(4), cubeVal4);
    vertlist[9] = vertexInterp(isolevel, cubePos(1), cubeVal1, cubePos(5), cubeVal5);
    vertlist[10] = vertexInterp(isolevel, cubePos(2), cubeVal2, cubePos(6), cubeVal6);
    vertlist[11] = vertexInterp(isolevel, cubePos(3), cubeVal3, cubePos(7), cubeVal7);

    // Create the triangle
    /////// gl_FrontColor=vec4(cos(isolevel*10.0-0.5), sin(isolevel*10.0-0.5), cos(1.0-isolevel),1.0);
    int i=0;
//    for (i=0; triTableValue(cubeindex, i) != -1 && i < 16; i+=3) { //Strange bug with this way, uncomment to test
    while(true){
        if(triTableValue(cubeindex, i)!=-1){
            //Generate first vertex of triangle//
            //Fill position varying attribute for fragment shader
            position= vec4(vertlist[triTableValue(cubeindex, i)], 1);
            //Fill gl_Position attribute for vertex raster space position
            gl_Position = proj_matrix * mv_matrix * position;
            ginitialVertPos = position.xyz;
            grealNormal = realNormal[0];
//            outColor = vec4(1.0, 0.0, 0.0, 1.0);
            EmitVertex();

            //Generate second vertex of triangle//
            //Fill position varying attribute for fragment shader
            position= vec4(vertlist[triTableValue(cubeindex, i+1)], 1);
            //Fill gl_Position attribute for vertex raster space position
            gl_Position = proj_matrix * mv_matrix * position;
            ginitialVertPos = position.xyz;
            grealNormal = realNormal[0];
//            outColor = vec4(0.0, 1.0, 0.0, 1.0);
            EmitVertex();

            //Generate last vertex of triangle//
            //Fill position varying attribute for fragment shader
            position= vec4(vertlist[triTableValue(cubeindex, i+2)], 1);
            //Fill gl_Position attribute for vertex raster space position
            gl_Position = proj_matrix * mv_matrix * position;
            ginitialVertPos = position.xyz;
            grealNormal = realNormal[0];
//            outColor = vec4(0.0, 0.0, 1.0, 1.0);
            EmitVertex();

            //End triangle strip at firts triangle
            EndPrimitive();
        }else{
            break;
        }

        i=i+3; //Comment it for testing the strange bug
    }

}
