/**
 * Do not modify this file.
 * This file contains the main code for applying Gaussian blur to an image.
 * It will call the functions you implemented in apply_gb_fast.c.
 */

#include "main.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void normalize_FVec(FVec v)
{
    unsigned int i,j;
    int ext = v.length / 2;
    v.sum[0] = v.data[ext];
    for (i = ext+1,j=1; i < v.length; i++,j++)
    {
        v.sum[j] = v.sum[j-1] + v.data[i]*2;
    }

}

float* get_pixel(Image img, int x, int y)
{
    if (x < 0)
    {
        x = 0;
    }
    if (x >= img.dimX)
    {
        x = img.dimX - 1;
    }
    if (y < 0)
    {
        y = 0;
    }
    if (y >= img.dimY)
    {
        y = img.dimY - 1;
    }
    return img.data + img.numChannels * (y * img.dimX + x);
}

float gd(float a, float b, float x)
{
    float c = (x-b) / a;
    return exp((-.5) * c * c) / (a * sqrt(2 * PI));
}

FVec make_gv(float a, float x0, float x1, unsigned int length, unsigned int min_length)
{
    FVec v;
    v.length = length;
    v.min_length = min_length;
    if(v.min_length > v.length){
        v.min_deta = 0;
    }else{
        v.min_deta = ((v.length - v.min_length) / 2);
    }
    v.data = malloc(length * sizeof(float));
    v.sum = malloc((length / 2 + 1)* sizeof(float));
    float step = (x1 - x0) / ((float)length);
    int offset = length/2;

    for (int i = 0; i < length; i++)
    {
        v.data[i] = gd(a, 0.0f, (i-offset)*step);
    }
    normalize_FVec(v);
    return v;
}

void print_fvec(FVec v)
{
    unsigned int i;
    printf("\n");
    for (i = 0; i < v.length; i++)
    {
        printf("%f ", v.data[i]);
    }
    printf("\n");
}

Image img_sc(Image a)
{
    Image b = a;
    b.data = malloc(b.dimX * b.dimY * b.numChannels * sizeof(float));
    return b;
}

Image gb_h(Image a, FVec gv)
{
    Image b = img_sc(a);

    int ext = gv.length / 2;
    int offset;
    unsigned int x, y, channel;
    float *pc;
    float sum;
    int i;
    for (channel = 0; channel < a.numChannels; channel++)
    {
        for (x = 0; x < a.dimX; x++)
        {
            for (y = 0; y < a.dimY; y++)
            {
                pc = get_pixel(b, x, y);
                unsigned int deta = fmin(fmin(a.dimY-y-1, y),fmin(a.dimX-x-1, x));
                deta = fmin(deta, gv.min_deta);
                sum = 0;
                for (i = deta; i < gv.length-deta; i++)
                {
                    offset = i - ext;
                    sum += gv.data[i]/gv.sum[ext - deta] * (float)get_pixel(a, x + offset, y)[channel];
                }
                pc[channel] = sum;
            }
        }
    }

    return b;
}

Image gb_v(Image a, FVec gv)
{
    Image b = img_sc(a);

    int ext = gv.length / 2;
    int offset;
    unsigned int x, y, channel;
    float* pc;
    float sum;
    int i;
    for (channel = 0; channel < a.numChannels; channel++)
    {
        for (x = 0; x < a.dimX; x++)
        {
            for (y = 0; y < a.dimY; y++)
            {
                pc = get_pixel(b, x, y);
                unsigned int deta = fmin(fmin(a.dimY-y-1, y),fmin(a.dimX-x-1, x));
                deta = fmin(deta, gv.min_deta);
                sum = 0;
                for (i = deta; i < gv.length-deta; i++)
                {
                    offset = i - ext;
                    sum += gv.data[i] /gv.sum[ext - deta] * (float)get_pixel(a, x, y + offset)[channel];
                }
                pc[channel] = sum;
            }
        }
    }
    return b;
}


int main(int argc, char** argv)
{
    struct timeval start_time, stop_time, elapsed_time; 
    gettimeofday(&start_time,NULL);
    if (argc < 6)
    {
        printf("Usage: ./gb.exe <inputjpg> <outputname> <float: a> <float: x0> <float: x1> <unsigned int: dim>\n");
        exit(0);
    }

    float a, x0, x1;
    unsigned int dim, min_dim;

    sscanf(argv[3], "%f", &a);
    sscanf(argv[4], "%f", &x0);
    sscanf(argv[5], "%f", &x1);
    sscanf(argv[6], "%u", &dim);
    sscanf(argv[7], "%u", &min_dim);

    FVec v = make_gv(a, x0, x1, dim, min_dim);
    // print_fvec(v);
    Image img;
    img.data = stbi_loadf(argv[1], &(img.dimX), &(img.dimY), &(img.numChannels), 0);

    Image imgOut = apply_gb(img, v);
    // Image imgOut = transpose(img);
    stbi_write_jpg(argv[2], imgOut.dimX, imgOut.dimY, imgOut.numChannels, imgOut.data, 90);
    gettimeofday(&stop_time,NULL);
    timersub(&stop_time, &start_time, &elapsed_time); 
    printf("%f \n", elapsed_time.tv_sec+elapsed_time.tv_usec/1000000.0);
    free(imgOut.data);
    free(v.data);
    free(v.sum);
    return 0;
}