#pragma once
#include "helper.h"


Eigen::Matrix4f camera_matrix(Vec3f camera_point, Vec3f eye_point, Vec3f up) {
    Vec3f g = (camera_point - eye_point).normalize();
    Vec3f x = (up ^ g).normalize();
    Vec3f y = (g ^ x).normalize();
    Eigen::Matrix<float, 4, 4> r_view;
    // ��������ϵΪ������ϵ��������ת������Խ���1��0��0�������磩�䵽x
    r_view <<
        x.x, y.x, g.x, 0,
        x.y, y.y, g.y, 0,
        x.z, y.z, g.z, 0,
        0, 0, 0, 1;
    Eigen::Matrix4f r_view_inv = r_view.inverse();
    Eigen::Matrix<float, 4, 4> t_view;
    t_view <<
        1, 0, 0, -camera_point.x,
        0, 1, 0, -camera_point.y,
        0, 0, 1, -camera_point.z,
        0, 0, 0, 1;
    return r_view_inv * t_view;
}

Eigen::Matrix4f perspective_matrix() {
    float l = -0.5, r = 0.5, b = -0.5, t = 0.5, n = -0.5, f = -5.5;
    Eigen::Matrix<float, 4, 4> p1;
    p1 <<
        n, 0, 0, 0,
        0, n, 0, 0,
        0, 0, n + f, -n * f,
        0, 0, 1, 0;
    Eigen::Matrix<float, 4, 4> o1;
    o1 <<
        2 / (r - l), 0, 0, 0,
        0, 2 / (t - b), 0, 0,
        0, 0, 2 / (n - f), 0,
        0, 0, 0, 1;
    Eigen::Matrix<float, 4, 4> o2;
    o2 <<
        1, 0, 0, -(r + l) / 2,
        0, 1, 0, -(t + b) / 2,
        0, 0, 1, -(n + f) / 2,
        0, 0, 0, 1;
    Eigen::Matrix<float, 4, 4> tem = o1 * o2;
    Eigen::Matrix<float, 4, 4> res = tem * p1;

    return res;
}

Vec3f point2vp(Eigen::Matrix<float, 4, 4> v, Eigen::Matrix<float, 4, 4> p, Vec3f point) {
    Vector4f point_hom(point.x, point.y, point.z, 1.);

    point_hom = v * point_hom;
    /*cout << p << endl << endl << point_hom << endl << endl;*/
    point_hom = p * point_hom;
    //cout << point_hom << endl << endl;
    point_hom = point_hom / point_hom[3];
    //cout << point_hom << endl << endl;
    return Vec3f(point_hom[0], point_hom[1], point_hom[2]);
}

float compute_w(Eigen::Matrix<float, 4, 4> v, Eigen::Matrix<float, 4, 4> p, Vec3f point) {
    Vector4f point_hom(point.x, point.y, point.z, 1.);

    point_hom = v * point_hom;
    point_hom = p * point_hom;

    return point_hom[3];
}

void line(Vec2f p1, Vec2f p2, TGAImage& image, TGAColor color) {
    // Bresenham�㷨��ֱ����x������ÿ������Ϊ\Delta x=1����y������ÿ�ε�����Ϊ\Delta y=k��
    //ͨ��һ������d��y�����ϵ��ۼ�������¼��������d����1ʱ����ǵ�m��1�����Ա���d����-1��
    //��ʹ��d�ķ�Χ��Զ������[0,1]֮�䡣������d�ķ�Χȷ�����յ�yֵ����d\leq 0.5ʱ����y���ֲ��䣻��d>0.5ʱ��y��1
    bool exchange = 0;
    int x0 = p1.x, y0 = p1.y, x1 = p2.x, y1 = p2.y;
    // ����ϵxy������ʹkΪ-1 - 1�����¼�����ʱ��Ҫ����ϵ����
    if (abs(y1 - y0) > abs(x1 - x0)) {
        swap(x1, y1);
        swap(x0, y0);
        exchange = 1;
    }

    // ���˳����Ҫ�������¼
    if (x1 < x0) {
        swap(x1, x0);
        swap(y0, y1);
    }
    int dx = x1 - x0, dy = y1 - y0;
    float d = 0;
    int y = y0;
    for (int x = x0; x <= x1; x++) {
        if (exchange) {
            image.set(y, x, color);
        }
        else {
            image.set(x, y, color);
        }
        // ��ؼ��������*2dx��������������Ч��
        d += abs(2 * dy);
        // ��Ϊ���ص������Ķ��������½Ƕ��㣬���Գ���0.5�����ƣ��Ұ������ĵ��²���y�����Ҫ����0.5֮��+-1
        if (d > dx) {
            y1 > y0 ? y += 1 : y -= 1;
            d -= 2 * dx;
        }
    }
}


Vec3f barycentric(Vec2f v1, Vec2f v2, Vec2f v3, Vec2f p)
{
    //���˷�ĸ����0�����
    if ((-(v1.x - v2.x) * (v3.y - v2.y) + (v1.y - v2.y) * (v3.x - v2.x)) == 0)
        return Vec3f(1, 0, 0);
    if (-(v2.x - v3.x) * (v1.y - v3.y) + (v2.y - v3.y) * (v1.x - v3.x) == 0)
        return Vec3f(1, 0, 0);
    float alpha = (-(p.x - v2.x) * (v3.y - v2.y) + (p.y - v2.y) * (v3.x - v2.x)) / (-(v1.x - v2.x) * (v3.y - v2.y) + (v1.y - v2.y) * (v3.x - v2.x));
    float beta = (-(p.x - v3.x) * (v1.y - v3.y) + (p.y - v3.y) * (v1.x - v3.x)) / (-(v2.x - v3.x) * (v1.y - v3.y) + (v2.y - v3.y) * (v1.x - v3.x));
    float gamma = 1 - alpha - beta;

    return Vec3f(alpha, beta, gamma);
}

bool IsInsideTriangle(Vec2f A, Vec2f B, Vec2f C, Vec2f p)
{
    double AB = (B.x - A.x) * (p.y - A.y) - (B.y - A.y) * (p.x - A.x);
    double BC = (C.x - B.x) * (p.y - B.y) - (C.y - B.y) * (p.x - B.x);
    double CA = (A.x - C.x) * (p.y - C.y) - (A.y - C.y) * (p.x - C.x);

    if (((AB > 0 && BC > 0 && CA > 0) || (AB < 0 && BC < 0 && CA < 0)))
        return true;
    else
        return false;
}

vector<vector<float>> msaa_check(Vec2f A, Vec2f B, Vec2f C, Vec2f p)
{
    vector<vector<float>>msaa_buffer;
    for (float i = -0.25; i <= 0.25; i += 0.5) {
        for (float j = -0.25; j <= 0.25; j += 0.5) {
            double AB = (B.x - A.x) * (p.y + i - A.y) - (B.y - A.y) * (p.x + j - A.x);
            double BC = (C.x - B.x) * (p.y + i - B.y) - (C.y - B.y) * (p.x + j - B.x);
            double CA = (A.x - C.x) * (p.y + i - C.y) - (A.y - C.y) * (p.x + j - C.x);

            if (((AB > 0 && BC > 0 && CA > 0) || (AB < 0 && BC < 0 && CA < 0))) {
                msaa_buffer.push_back({ 1, i, j });
            }
            else {
                msaa_buffer.push_back({ 0, i, j });
            }
        }
    }
    return msaa_buffer;
}

//�������ϸ��ͷ
void drawAfrican(Model* model, TGAImage& image, Vec3f light_dir, int width, int height,
    Eigen::Matrix4f camera_matrix, Eigen::Matrix4f p_matrix, Vec3f camera_point) {

    ComputeData* pgpu_compute_data =new ComputeData(width, height, camera_point, light_dir);
   
    int nfaces = model->nfaces();
    allocator<VertexData>alloc;
    VertexData* pgpu_vertex_data = alloc.allocate(nfaces);

    for (int i = 0; i < nfaces; i++) {
        std::vector<int> face = model->face(i);
        Vec3f p1_prior = model->vert(face[0]);
        Vec3f p2_prior = model->vert(face[1]);
        Vec3f p3_prior = model->vert(face[2]);
        Vec2i uv1(0, 0);
        Vec2i uv2(0, 0);
        Vec2i uv3(0, 0);
        uv1 = model->uv(i, 0);
        uv2 = model->uv(i, 1);
        uv3 = model->uv(i, 2);
        Vec3f n1 = model->normal(i, 0);
        Vec3f n2 = model->normal(i, 1);
        Vec3f n3 = model->normal(i, 2);
        
        Vec3f p1 = point2vp(camera_matrix, p_matrix, p1_prior);
        Vec3f p2 = point2vp(camera_matrix, p_matrix, p2_prior);
        Vec3f p3 = point2vp(camera_matrix, p_matrix, p3_prior);

        float w1 = compute_w(camera_matrix, p_matrix, p1_prior);
        float w2 = compute_w(camera_matrix, p_matrix, p2_prior);
        float w3 = compute_w(camera_matrix, p_matrix, p3_prior);
        
        vector<Vec3f>ps{ p1,p2,p3 };
        vector<Vec2i>uvs{ uv1,uv2,uv3 };
        vector<float>ws{ w1,w2,w3 };
        vector<Vec3f>ps_prior{ p1_prior,p2_prior,p3_prior };
        vector<Vec3f>ns{ n1, n2, n3 };

        alloc.construct(pgpu_vertex_data+i, ps, uvs, ws, ps_prior, ns);
    }
    GputoCpuData* gputocpu = new GputoCpuData();
    GputoCpuData* res = gpu_memcpy(pgpu_vertex_data, pgpu_compute_data, gputocpu, nfaces, scale, scale);
    for (int i = 0; i < scale; i++) {
        for (int j = 0; j < scale; j++) {
            int id = i * scale + j;
            if (res->idxs[id] == true) {
                float indensity = res->colorbuffer_indensity[id];
                float blinphong = res->colorbuffer_blinphong[id];

                TGAColor color = model->diffuse(Vec2i(res->colorbuffer_x[id], res->colorbuffer_y[id]));

                unsigned char r = min(int(color.r * (indensity + 0.2 + blinphong)), 255);
                unsigned char g = min(int(color.g * (indensity + 0.2 + blinphong)), 255);
                unsigned char b = min(int(color.b * (indensity + 0.2 + blinphong)), 255);

                TGAColor res_color = TGAColor(r, g, b, 255);
                image.set(j, i, res_color);
            }
        }
    }
    delete gputocpu;
    delete pgpu_compute_data;
}