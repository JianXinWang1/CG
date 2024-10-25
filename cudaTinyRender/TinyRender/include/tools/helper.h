#pragma once
#include "tgaimage.h"
#include <iostream>
#include "model.h"
#include <vector>
#include <Eigen/Dense>
#include<Eigen/Core>

#include<cudnn.h>

const int scale = 200;
using namespace Eigen;
using namespace std;

struct GputoCpuData {
    float colorbuffer_x[scale * scale];
    float colorbuffer_y[scale * scale];
    float colorbuffer_indensity[scale * scale];
    float colorbuffer_blinphong[scale * scale];
    bool idxs[scale * scale];
};

struct ComputeData {
    ComputeData(int width, int height,  Vec3f camera_point, Vec3f light_dir) {
        std::fill(this->depthbuffer, this->depthbuffer + scale * scale, -6.);

        this->camera_point[0] = camera_point[0];
        this->camera_point[1] = camera_point[1];
        this->camera_point[2] = camera_point[2];

        this->light_dir[0] = light_dir[0];
        this->light_dir[1] = light_dir[1];
        this->light_dir[2] = light_dir[2];
    }
    float depthbuffer[scale * scale];
    float camera_point[3];
    float light_dir[3];
};

struct VertexData {
    VertexData(vector<Vec3f>ps, vector<Vec2i>uvs, vector<float>ws, vector<Vec3f>ps_prior, vector<Vec3f>ns) {
        this->mvp_ps[0][0] = ps[0].x;
        this->mvp_ps[0][1] = ps[0].y;
        this->mvp_ps[0][2] = ps[0].z;
        this->mvp_ps[1][0] = ps[1].x;
        this->mvp_ps[1][1] = ps[1].y;
        this->mvp_ps[1][2] = ps[1].z;
        this->mvp_ps[2][0] = ps[2].x;
        this->mvp_ps[2][1] = ps[2].y;
        this->mvp_ps[2][2] = ps[2].z;

        this->uvs[0][0] = uvs[0].x;
        this->uvs[0][1] = uvs[0].y;
        this->uvs[1][0] = uvs[1].x;
        this->uvs[1][1] = uvs[1].y;
        this->uvs[2][0] = uvs[2].x;
        this->uvs[2][1] = uvs[2].y;

        this->ws[0] = ws[0];
        this->ws[1] = ws[1];
        this->ws[2] = ws[2];

        this->model_ps[0][0] = ps_prior[0].x;
        this->model_ps[0][1] = ps_prior[0].y;
        this->model_ps[0][2] = ps_prior[0].z;
        this->model_ps[1][0] = ps_prior[1].x;
        this->model_ps[1][1] = ps_prior[1].y;
        this->model_ps[1][2] = ps_prior[1].z;
        this->model_ps[2][0] = ps_prior[2].x;
        this->model_ps[2][1] = ps_prior[2].y;
        this->model_ps[2][2] = ps_prior[2].z;

        this->ns[0][0] = ns[0].x;
        this->ns[0][1] = ns[0].y;
        this->ns[0][2] = ns[0].z;
        this->ns[1][0] = ns[1].x;
        this->ns[1][1] = ns[1].y;
        this->ns[1][2] = ns[1].z;
        this->ns[2][0] = ns[2].x;
        this->ns[2][1] = ns[2].y;
        this->ns[2][2] = ns[2].z;
    }
    float mvp_ps[3][3];
    float uvs[3][2];
    float ws[3];
    float model_ps[3][3];
    float ns[3][3];
};


GputoCpuData* gpu_memcpy(VertexData * pgpudata, ComputeData * pgpu_computedata, GputoCpuData* pgputocpu, int n, int width, int height);
Eigen::Matrix4f camera_matrix(Vec3f camera_point, Vec3f eye_point, Vec3f up);
Eigen::Matrix4f perspective_matrix();
Vec3f point2vp(Eigen::Matrix<float, 4, 4> v, Eigen::Matrix<float, 4, 4> p, Vec3f point);
float compute_w(Eigen::Matrix<float, 4, 4> v, Eigen::Matrix<float, 4, 4> p, Vec3f point);
void line(Vec2f p1, Vec2f p2, TGAImage& image, TGAColor color);
Vec3f barycentric(Vec2f v1, Vec2f v2, Vec2f v3, Vec2f p);
vector<vector<float>> msaa_check(Vec2f A, Vec2f B, Vec2f C, Vec2f p);

// 光栅化、着色函数
void triangle_render(Model* model, vector<Vec3f>ps, vector<Vec2i>uvs, TGAImage& image, vector<float>& buffer,
    int weith, int height, vector<Vec3f>ns, Vec3f camera_point, Vec3f light_dir,
    vector<Vec3f>ps_prior, vector<float>ws);

void drawAfrican(Model* model, TGAImage& image, Vec3f light, int weith, int height,
    Eigen::Matrix4f camera_matrix, Eigen::Matrix4f p_matrix, Vec3f camera_point);


