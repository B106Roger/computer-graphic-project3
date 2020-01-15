#pragma once
// ****************************************************************************
// Reference From https://www.itread01.com/content/1546649662.html  ***********
// ****************************************************************************

#ifndef TGATEXTURE_H
#define TGATEXTURE_H

#include <QtOpenGL/QtOpenGL>
#include <iostream>

using namespace std;

//���z���c��w�q
typedef struct
{
	GLubyte *imageData;//�v�H���
	GLuint bpp;//�e���`��
	GLuint width;//�v�H�e��
	GLuint height;//�v�H����
	GLuint texID;//���������zID
} TextureImage;

//���JTGA�v�H�A�ͦ����z
bool LoadTGA(TextureImage *texture, char *fileName);
//���JTGA skybox
bool LoadTGAData(TextureImage *texture, char *fileName);

void *file_contents(const char *filename, GLint *length);
short le_short(unsigned char *bytes);
void *read_tga(const char *filename, int *width, int *height);

#endif