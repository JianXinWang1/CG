#include "helper.h"
#include "helper_math.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <cuda_runtime_api.h>

inline __host__ __device__ float* barycentric_gpu(float* A, float* B, float* C, float* p)
{
    float* res = new float[3];
    //别丢了分母等于0的情况
    if ((-(A[0] - B[0]) * (C[1] - B[1]) + (A[1] - B[1]) * (C[0] - B[0])) == 0) {
        res[0] = 1.0;
        res[1] = 0.0;
        res[2] = 0.0;
        return res;
    }
    if (-(B[0] - C[0]) * (A[1] - C[1]) + (B[1] - C[1]) * (A[0] - C[0]) == 0) {
        res[0] = 1.0;
        res[1] = 0.0;
        res[2] = 0.0;
        return res;
    }
    float alpha = (-(p[0] - B[0]) * (C[1] - B[1]) + (p[1] - B[1]) * (C[0] - B[0])) / (-(A[0] - B[0]) * (C[1] - B[1]) + (A[1] - B[1]) * (C[0] - B[0]));
    float beta = (-(p[0] - C[0]) * (A[1] - C[1]) + (p[1] - C[1]) * (A[0] - C[0])) / (-(B[0] - C[0]) * (A[1] - C[1]) + (B[1] - C[1]) * (A[0] - C[0]));
    float gamma = 1 - alpha - beta;

    res[0] = alpha;
    res[1] = beta;
    res[2] = gamma;
    return res;
}

__global__ void gpushaderkernel(VertexData* fp_vertex_device, ComputeData* fp_compute_device, GputoCpuData* pgputocpu, int n, int width, int height) {
	const int id = blockIdx.x * blockDim.x + threadIdx.x;
    if (id >= n) {
        return;
    }
    
    float (*ps)[3] = fp_vertex_device[id].mvp_ps;
    float* ws = fp_vertex_device[id].ws;
    float(*ps_prior)[3] = fp_vertex_device[id].model_ps;
    float(*ns)[3] = fp_vertex_device[id].ns;
    float(*uvs)[2] = fp_vertex_device[id].uvs;

    float* depthbuffer = fp_compute_device->depthbuffer;
    float* camera_point = fp_compute_device->camera_point;
    float* light_dir = fp_compute_device->light_dir;

    float* colorbuffer_x = pgputocpu->colorbuffer_x;
    float* colorbuffer_y = pgputocpu->colorbuffer_y;
    float* colorbuffer_indensity = pgputocpu->colorbuffer_indensity;
    float* colorbuffer_blinphong = pgputocpu->colorbuffer_blinphong;
    bool* idxs = pgputocpu->idxs;


    // 到图像坐标
    float x1 = (ps[0][0] + 1.) * width / 2., y1 = (ps[0][1] + 1.) * height / 2.;
    float x2 = (ps[1][0] + 1.) * width / 2., y2 = (ps[1][1] + 1.) * height / 2.;
    float x3 = (ps[2][0] + 1.) * width / 2., y3 = (ps[2][1] + 1.) * height / 2.;

    //printf("%f %f %f %f %f \n", ps[0][0], ws[0], ps_prior[0][0], ns[0][0], uvs[2][0]);

    // 图像坐标系包围框
    float x_min = min(x1, x2);
    x_min = min(x_min, x3);

    float x_max = max(x1, x2);
    x_max = max(x_max, x3);

    float y_min = min(y1, y2);
    y_min = min(y_min, y3);

    float y_max = max(y1, y2);
    y_max = max(y_max, y3);

    for (int x = x_min - 1; x <= x_max + 1; x++) {
        for (int y = y_min - 1; y <= y_max + 1; y++) {
            if (x < 0 || x > width || y < 0 || y > height) {
                continue;
            }
            float A[2] = { x1, y1 };
            float B[2] = { x2, y2 };
            float C[2] = { x3, y3 };
            float p[2] = { x + 0.5, y + 0.5 };
            float AB = (B[0] - A[0]) * (p[1] - A[1]) - (B[1] - A[1]) * (p[0] - A[0]);
            float BC = (C[0] - B[0]) * (p[1] - B[1]) - (C[1] - B[1]) * (p[0] - B[0]);
            float CA = (A[0] - C[0]) * (p[1] - C[1]) - (A[1] - C[1]) * (p[0] - C[0]);
            if ((AB > 0 && BC > 0 && CA > 0) || (AB < 0 && BC < 0 && CA < 0)) {
                float* bary_centric = barycentric_gpu(A, B, C, p);
                // 透视矫正
                float k = 1.0 / (bary_centric[0] * ws[0] + bary_centric[1] * ws[1] + bary_centric[2] * ws[2]);
                float alpha = bary_centric[0] / (ws[0] * k);
                float beta = bary_centric[1] / (ws[1] * k);
                float gama = bary_centric[2] / (ws[2] * k);

                float p_depth = (alpha * ps[0][2] + beta * ps[1][2] + gama * ps[2][2]) / 2.0 + 0.5;
                int idx = y * height + x;
                if (depthbuffer[idx] < p_depth) {
                    depthbuffer[idx] = p_depth;

                    float3 ps_prior1_cu = make_float3(ps_prior[0][0], ps_prior[0][1], ps_prior[0][2]);
                    float3 ps_prior2_cu = make_float3(ps_prior[1][0], ps_prior[1][1], ps_prior[1][2]);
                    float3 ps_prior3_cu = make_float3(ps_prior[2][0], ps_prior[2][1], ps_prior[2][2]);
                    float3 interpertion_point = ps_prior1_cu * alpha + ps_prior2_cu * beta + ps_prior3_cu * gama;

                    float3 camera_point_cu = make_float3(camera_point[0], camera_point[1], camera_point[2]);
                    float3 eye_light = camera_point_cu - interpertion_point;
                    eye_light = normalize(eye_light);

                    float3 n1_cu = normalize(make_float3(ns[0][0], ns[0][1], ns[0][2]));
                    float3 n2_cu = normalize(make_float3(ns[1][0], ns[1][1], ns[1][2]));
                    float3 n3_cu = normalize(make_float3(ns[2][0], ns[2][1], ns[2][2]));

                    float3 normal = n1_cu * alpha + n2_cu * beta + n3_cu * gama;
                    normal = normalize(normal);

                    float3 light_dir_cu = normalize(make_float3(light_dir[0], light_dir[1], light_dir[2]));
                    float3 half_v = eye_light + light_dir_cu;
                    half_v = normalize(half_v);

                    float blin_phong = pow(max(0.f, dot(half_v, normal)), 16);
                    float indensity = max(0.f, dot(normal, light_dir_cu));

                    float2 color_index_cu = make_float2(alpha * uvs[0][0] + beta * uvs[1][0] + gama * uvs[2][0], alpha * uvs[0][1] + beta * uvs[1][1] + gama * uvs[2][1]);

                    colorbuffer_x[idx] = color_index_cu.x;
                    colorbuffer_y[idx] = color_index_cu.y;
                    colorbuffer_indensity[idx] = indensity;
                    colorbuffer_blinphong[idx] = blin_phong;
                    idxs[idx] = true;
                    //printf("%d %f %f %f %f\n", idx, color_index_cu.x, color_index_cu.y, indensity, blin_phong);
                }
            }
        }
    }

}

GputoCpuData* gpu_memcpy(VertexData* pgpu_vertex_data, ComputeData* pgpu_computedata, GputoCpuData* pgputocpu, int n, int width, int height) {
    cudaSetDevice(0);

    VertexData* fp_vertex_device;
    cudaMalloc((void**)&fp_vertex_device, n * sizeof(VertexData));

    ComputeData* fp_compute_device;
    cudaMalloc((void**)&fp_compute_device, sizeof(ComputeData));

    GputoCpuData* fp_gputocpu;
    cudaMalloc((void**)&fp_gputocpu, sizeof(GputoCpuData));

    cudaMemcpy(fp_vertex_device, pgpu_vertex_data, n * sizeof(VertexData), cudaMemcpyHostToDevice);
    cudaMemcpy(fp_compute_device, pgpu_computedata, sizeof(ComputeData), cudaMemcpyHostToDevice);
    auto stat = cudaMemcpy(fp_gputocpu, pgputocpu, sizeof(GputoCpuData), cudaMemcpyHostToDevice);

    int threads = 256, blocks = (n + threads - 1) / threads;

    gpushaderkernel << <blocks, threads >> > (fp_vertex_device, fp_compute_device, fp_gputocpu, n, width, height);
    cudaDeviceSynchronize();

    GputoCpuData res_gputocpu;
    stat = cudaMemcpy(&res_gputocpu,  fp_gputocpu, sizeof(GputoCpuData), cudaMemcpyDeviceToHost);

    return &res_gputocpu;
}
