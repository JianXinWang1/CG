#include "tgaimage.h"
#include "helper.h"

// 全局变量：光照，相机位置等
Vec3f light_point(0.5, 0.5, 1);
Vec3f eye_point(0, 0, 0);
Vec3f light_dir = light_point - eye_point;
Vec3f camera_point(-0.4, -0.1, 1.3);

int weith = 2000, height = 2000;
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
int main(int argc, char** argv) {
	
    Model *model = new Model("obj/african_head.obj");
    TGAImage image(weith, height, TGAImage::RGB);
	Eigen::Matrix4f c2w_matrix = camera_matrix(camera_point, eye_point, Vec3f(0, 1, 0));
	Eigen::Matrix4f p_matrix = perspective_matrix();
	drawAfrican(model, image,light_dir, weith, height,c2w_matrix,p_matrix, camera_point);

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
    delete model;
	return 0;
}