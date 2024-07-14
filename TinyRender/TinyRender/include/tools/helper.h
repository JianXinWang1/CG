#pragma once
#include "tgaimage.h"
#include <iostream>
#include "model.h"
#include <vector>
#include <Eigen/Dense>
#include<Eigen/Core>

using namespace Eigen;
using namespace std;

Eigen::Matrix4f camera_matrix(Vec3f camera_point, Vec3f eye_point, Vec3f up) {
    // 
    Vec3f g = (camera_point-eye_point).normalize();
    Vec3f x = (up^g).normalize();
    Vec3f y = (g^x).normalize();
    Eigen::Matrix<float,4,4> r_view;
    // ��������ϵΪ������ϵ��������ת������Խ���1��0��0�������磩�䵽x
    r_view <<
        x.x, y.x, g.x, 0,
        x.y, y.y, g.y, 0,
        x.z, y.z, g.z, 0,
        0, 0, 0, 1;
    Eigen::Matrix4f r_view_inv = r_view.inverse();
    Eigen::Matrix<float,4,4> t_view;
    t_view <<
        1, 0, 0, -camera_point.x,
        0, 1, 0, -camera_point.y,
        0, 0, 1, -camera_point.z,
        0, 0, 0, 1;
    return r_view_inv * t_view;

}


Eigen::Matrix4f perspective_matrix() {
    float l = -0.5, r = 0.5, b = -0.5 ,t = 0.5, n = -0.5, f = -3.5;
    Eigen::Matrix<float, 4, 4> p1;
    p1 <<
        n, 0, 0, 0,
        0, n, 0, 0,
        0, 0, n + f, -n * f,
        0, 0, 1, 0;
    Eigen::Matrix<float, 4, 4> o1;
    o1 <<
        2/(r-l), 0, 0, 0,
        0, 2/(t-b), 0, 0,
        0, 0, 2 / (n - f), 0,
        0, 0,0, 1;
    Eigen::Matrix<float, 4, 4> o2;
    o2 <<
        1, 0, 0,-(r+l)/2,
        0, 1, 0, -(t+b)/2,
        0, 0, 1, -(n + f)/2,
        0, 0, 0, 1;
    Eigen::Matrix<float, 4, 4> tem = o1 * o2;
    Eigen::Matrix<float, 4, 4> res = tem*p1;

    return res;

}


Vec3f point2vp(Eigen::Matrix<float,4,4> v, Eigen::Matrix<float, 4, 4> p, Vec3f point) {
    Vector4f point_hom(point.x, point.y, point.z, 1.);
   
    point_hom = v * point_hom;
    /*cout << p << endl << endl << point_hom << endl << endl;*/
    point_hom = p * point_hom;
    //cout << point_hom << endl << endl;
    point_hom =  point_hom/point_hom[3];
    //cout << point_hom << endl << endl;
    return Vec3f(point_hom[0], point_hom[1], point_hom[2]);
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
        d += abs(2*dy);
        // ��Ϊ���ص������Ķ��������½Ƕ��㣬���Գ���0.5�����ƣ��Ұ������ĵ��²���y�����Ҫ����0.5֮��+-1
        if (d > dx) {
            y1 > y0 ? y += 1 : y -= 1;
            d -= 2*dx;
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


void triangle_render(Model* model,Vec3f p1, Vec3f p2, Vec3f p3,Vec2i uv1, Vec2i uv2, Vec2i uv3, TGAImage& image, vector<float>&buffer,
    int weith,int height, Vec3f n1, Vec3f n2, Vec3f n3, Vec3f camera_point,Vec3f light_dir,
    Vec3f p1_prior, Vec3f p2_prior, Vec3f p3_prior) {
     /*����*/
    /*line(p1, p2, image, color);
    line(p2, p3, image, color);
    line(p3, p1, image, color);*/
    // ģ������任����Ļ����
    float ambient = 0.1; // ������
    
    // ��ͼ������ϵ
    float x1 = (p1.x + 1.) * weith / 2., y1 = (p1.y + 1.) * height / 2. ;
    float x2 = (p2.x + 1.) * weith / 2. , y2 = (p2.y + 1.) * height / 2.;
    float x3 = (p3.x + 1.) * weith / 2. , y3 = (p3.y + 1.) * height / 2.;
    // ͼ������ϵ��Χ��
    float x_min = min(x1, x2);
    x_min = min(x_min, x3);

    float x_max = max(x1, x2);
    x_max = max(x_max, x3);

    float y_min = min(y1, y2);
    y_min = min(y_min, y3);

    float y_max = max(y1, y2);
    y_max = max(y_max, y3);


    
    // �ڲ����㣬�������귨���ߵ���жϷ�
    for (int x = x_min-1; x <= x_max+1; x++) {
        for (int y = y_min-1; y <= y_max+1; y++) {
            Vec3f bary_centric = barycentric(Vec2f(p1.x, p1.y), Vec2f(p2.x, p2.y), Vec2f(p3.x, p3.y), Vec2f((x+0.5)*2./weith-1., (y + 0.5) * 2. /height - 1.));
            if (bary_centric.x >= 0&& bary_centric.y >= 0 && bary_centric.z >= 0){
                int idx = y * weith + x;
                if (idx >= buffer.size()) { continue; }
                float p_depth = bary_centric.x * p1.z + bary_centric.y * p2.z + bary_centric.z * p3.z;
                Vec3f interpertion_point(bary_centric.x * p1_prior.x + bary_centric.y * p2_prior.x + bary_centric.z * p3_prior.x,
                    bary_centric.x * p1_prior.y + bary_centric.y * p2_prior.y + bary_centric.z * p3_prior.y,
                    bary_centric.x * p1_prior.z + bary_centric.y * p2_prior.z + bary_centric.z * p3_prior.z);
                Vec3f eye_light = camera_point - interpertion_point;
                eye_light.normalize();
                Vec3f normal(bary_centric.x * n1.x + bary_centric.y * n2.x + bary_centric.z * n3.x,
                    bary_centric.x * n1.y + bary_centric.y * n2.y + bary_centric.z * n3.y, 
                    bary_centric.x * n1.z + bary_centric.y * n2.z + bary_centric.z * n3.z);
                normal.normalize();
                light_dir.normalize();
                Vec3f half_v = (eye_light + light_dir);
                half_v.normalize();

                float blin_phong = pow(max(0.f, half_v * normal),16);

             
                // ����ָ���ķ������������ķ�����
                float indensity =max(0.f, normal * light_dir);


                // ��-z
                if (buffer[idx] < p_depth) {
                    buffer[idx] = p_depth;
                    Vec2i color_index = Vec2i(bary_centric.x * uv1.x + bary_centric.y * uv2.x + bary_centric.z * uv3.x,
                        bary_centric.x * uv1.y + bary_centric.y * uv2.y + bary_centric.z * uv3.y);
                    TGAColor color = model->diffuse(color_index);

                    unsigned char r = min(int(color.r * (indensity + ambient + blin_phong)), 255);
                    unsigned char g = min(int(color.g * (indensity + ambient + blin_phong)), 255);
                    unsigned char b = min(int(color.b * (indensity + ambient + blin_phong)), 255);

                    TGAColor res = TGAColor(r, g, b , 255);
                    image.set(x, y, res);
                }
            }
        }
    }

}

//�������ϸ��ͷ
void drawAfrican(Model* model, TGAImage& image,Vec3f light, int weith, int height,
    Eigen::Matrix4f camera_matrix, Eigen::Matrix4f p_matrix, Vec3f camera_point) {
    // ��Ȼ���
    vector<float>buffer(weith * height, -99999999999.);
    for (int i = 0; i < model->nfaces(); i++) {
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
        light.normalize();
        Vec3f p1 = point2vp(camera_matrix, p_matrix,  p1_prior);
        Vec3f p2 = point2vp(camera_matrix, p_matrix,p2_prior);
        Vec3f p3 = point2vp(camera_matrix, p_matrix, p3_prior);
        // ����ü�=���˷��ߺ͹��ռнǹ���,��Ⱦ��ǰ�棬�����ǰ�渲�ǵ���
        triangle_render(model,p1, p2, p3,uv1,uv2,uv3, image,  buffer,weith,height,n1,n2,n3,camera_point,light,p1_prior,p2_prior,p3_prior);

    }
}


