#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#define PI 3.14159265358979323846

// 定义图像结构体
typedef struct {
    int width;
    int height;
    int depth;
    int channel;
    unsigned char* data;
} Image;

// 函数声明
Image* load_image(const char* filename);
void save_image(Image* image, const char* filename);
void dct2d(double* input, double* output, int width, int height);
void idct2d(double* input, double* output, int width, int height);
void denoise_image(Image* image, double threshold);

int main() {
    Image* image = load_image("input.jpg");
    if (image == NULL) {
        printf("Error load image.\n");
        return 1;
    }

    denoise_image(image, 10.0);

    save_image(image, "output.jpg");

    free(image->data);
    free(image);

    return 0;
}

// 加载图像
Image* load_image(const char* filename) {
    Image* image = (Image*)malloc(sizeof(Image));
    if (image == NULL) {
        return NULL; // 内存分配失败
    }

    // 使用 stb_image 加载图像
    image->data = stbi_load(filename, &image->width, &image->height, &image->channel, 0);
    if (image->data == NULL) {
        free(image);
        return NULL; // 加载失败
    }

    // 设置深度，通常为 1（灰度图）或 3（RGB 图）或 4（RGBA 图）
    image->depth = 8; // 假设图像是 8 位深度
    return image;
}
void save_image(Image* image, const char* filename) {
    // 根据通道数选择函数
    if (image->channel == 3) {
        stbi_write_jpg(filename, image->width, image->height, image->channel, image->data, 100); // 质量为 100%
    }
    else if (image->channel == 1) {
        stbi_write_png(filename, image->width, image->height, image->channel, image->data, image->width);
    }
    // 对于其他通道数的处理可以根据需要添加
}
// 二维离散余弦变换
void dct2d(double* input, double* output, int width, int height) {
    int i, j, k, l;
    double sum;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            sum = 0.0;
            for (k = 0; k < height; k++) {
                for (l = 0; l < width; l++) {
                    sum += input[k * width + l] * cos((2 * k + 1) * i * PI / (2 * height)) * cos((2 * l + 1) * j * PI / (2 * width));
                }
            }
            output[i * width + j] = sum;
        }
    }
}

// 二维离散余弦逆变换
void idct2d(double* input, double* output, int width, int height) {
    int i, j, k, l;
    double sum;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            sum = 0.0;
            for (k = 0; k < height; k++) {
                for (l = 0; l < width; l++) {
                    sum += input[k * width + l] * cos((2 * i + 1) * k * PI / (2 * height)) * cos((2 * j + 1) * l * PI / (2 * width));
                }
            }
            output[i * width + j] = sum * 0.25; // 乘以 1/4
        }
    }
}

// 图像去噪
void denoise_image(Image* image, double threshold) {
    int width = image->width;
    int height = image->height;
    int depth = image->depth;
    int channel = image->channel;
    int i, j, k;

    // 为 DCT 变换分配内存
    double* dct_data = (double*)malloc(width * height * sizeof(double));
    if (dct_data == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    // 对每个通道进行 DCT 变换
    for (k = 0; k < channel; k++) {
        // 将图像数据复制到 DCT 数据缓冲区
        for (i = 0; i < height; i++) {
            for (j = 0; j < width; j++) {
                dct_data[i * width + j] = (double)image->data[i * width * channel + j * channel + k];
            }
        }

        // 进行二维 DCT 变换
        dct2d(dct_data, dct_data, width, height);

        // 设置阈值，将低于阈值的高频系数置零
        for (i = 0; i < height; i++) {
            for (j = 0; j < width; j++) {
                if (i + j > threshold) {
                    dct_data[i * width + j] = 0.0;
                }
            }
        }

        // 进行二维逆 DCT 变换
        idct2d(dct_data, dct_data, width, height);

        // 将去噪后的数据复制回图像缓冲区
        for (i = 0; i < height; i++) {
            for (j = 0; j < width; j++) {
                image->data[i * width * channel + j * channel + k] = (unsigned char)dct_data[i * width + j];
            }
        }
    }

    free(dct_data);
}
