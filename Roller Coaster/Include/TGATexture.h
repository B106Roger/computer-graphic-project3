#pragma once
// ****************************************************************************
// Reference From https://www.itread01.com/content/1546649662.html  ***********
// ****************************************************************************

#ifndef TGATEXTURE_H
#define TGATEXTURE_H

#include <QtOpenGL/QtOpenGL>
#include <iostream>

using namespace std;

//紋理結構體定義
typedef struct
{
	GLubyte *imageData;//影象資料
	GLuint bpp;//畫素深度
	GLuint width;//影象寬度
	GLuint height;//影象高度
	GLuint texID;//對應的紋理ID
} TextureImage;

//載入TGA影象，生成紋理
bool LoadTGA(TextureImage *texture, char *fileName);
//載入TGA skybox
bool LoadTGAData(TextureImage *texture, char *fileName);

void *file_contents(const char *filename, GLint *length);
short le_short(unsigned char *bytes);
void *read_tga(const char *filename, int *width, int *height);

#endif