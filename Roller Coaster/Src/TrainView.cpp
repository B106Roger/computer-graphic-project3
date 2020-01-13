#include "TrainView.h"  
#include <QtMultimedia/QMediaPlayer>
#include <assert.h>
#include "AppMain.h"

using namespace std;
TrainView::TrainView(QWidget *parent) :
	QGLWidget(parent)
{
	resetArcball();
}
TrainView::~TrainView()
{}
void TrainView::initializeGL()
{

	initializeOpenGLFunctions();

	//Create a triangle object
	triangle = new Triangle();
	triangle->Init();

	//Create a square object
	square = new Square();
	square->Init();

	//Create a water object
	water = new Water();
	water->Init();

	//Create a skybox object
	sky = new Skybox();
	sky->Init();
	Model::skyboxShaderID = sky->skyboxTextureID;


	//Initialize texture 
	initializeTexture();
	//Initialize music
	initializeMedia();

	//particle參數
	grav = 0.00003f;
	nOfFires = 0;
	Tick1 = Tick2 = GetTickCount();
	AppMain::getInstance()->ChangeSpeedOfTrain(50);
	Particles = NULL;

	// 軌道參數
	curve = 0;          // 軌道類別
	// 其他物件
	spaceShip = new Model("./Object/Transport_Shuttle_obj.obj", 20, Point3d(-6.f, 10.f, 3.f));
	/*spaceShipReflection = new Model("./Object/Transport_Shuttle_obj.obj", 20, Point3d(-6.f, 20.f, 3.f), REFLECTION);
	spaceShipRefraction = new Model("./Object/Transport_Shuttle_obj.obj", 20, Point3d(-6.f, 20.f, 3.f), REFRACTION);*/

	TrainItem item(0,0,true);
	item.train = new Model("./Object/Transport_Shuttle_obj.obj", 20, Point3d(-6.f, 20.f, 3.f), TRAIN);
	trainList.push_back(item);



	//Model test
	/*earth = new Model*[10];
	spaceTest = new Model*[10];
	for (int i = 0; i < 5; i++)
	{
		earth[i] = new Model("./Object/earth.obj", 50, Point3d(i * 1000, i * 1000, 0));
		spaceTest[i] = new Model("./Object/Kameriexplorerflying.obj", 20, Point3d(i * 1000, i * 1000, 0));
	}
	for (int i = 6; i < 10; i++)
	{
		earth[i] = new Model("./Object/earth.obj", 50, Point3d((i - 5) * -1000, (i - 5) * 1000, 0));
		spaceTest[i] = new Model("./Object/Kameriexplorerflying.obj", 20, Point3d((i - 5) * -1000, (i - 5) * 1000, 0));
	}*/

	// 初始化火車時間
	t_time = 0.f;
	// 初始化火車跑布林參數
	isrun = false;
	// 火車速度
	TRAIN_SPEED = 0.1f;
	// 火車軌道曲線
	vector<vector<float>>
		spline_Linear =
	{
		{1,0,0,0},
		{0,1,0,0},
		{0,0,1,0},
		{0,0,0,1},
	},
	spline_CardinalCubic =
	{
		{    -0.5,   1, -0.5,       0},
		{     1.5,-2.5,    0,       1},
		{    -1.5,   2,  0.5,       0},
		{     0.5,-0.5,    0,       0},
	},
	spline_CubicB_Spline =
	{
		{-1.f / 6, 0.5, -0.5, 1.f / 6},
		{     0.5,  -1,    0, 2.f / 3},
		{    -0.5, 0.5,  0.5, 1.f / 6},
		{ 1.f / 6,   0,    0,       0},
	};
	M_curve.push_back(spline_Linear);
	M_curve.push_back(spline_CardinalCubic);
	M_curve.push_back(spline_CubicB_Spline);
}
void TrainView::initializeTexture()
{
	//Load and create a texture for square;'stexture
	QOpenGLTexture* texture = new QOpenGLTexture(QImage("./Textures/Tupi.bmp"));
	Textures.push_back(texture);
}
void TrainView::resetArcball()
//========================================================================
{
	// Set up the camera to look at the world
	// these parameters might seem magical, and they kindof are
	// a little trial and error goes a long way
	arcball.setup(this, 40, 250, .2f, .4f, 0);
}

void TrainView::paintGL()
{
	this->printFPS();
	//*********************************************************************
	//
	// * Set up basic opengl informaiton
	//
	//**********************************************************************
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	// Set up the view port
	// 978, 486
	glViewport(0, 0, width(), height());
	// clear the window, be sure to clear the Z-Buffer too
	glClearColor(0, 0, 0.3f, 0);

	// we need to clear out the stencil buffer since we'll use
	// it for shadows
	glClearStencil(0);
	glEnable(GL_DEPTH);

	// Blayne prefers GL_DIFFUSE
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	// prepare for projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	setProjection();		// put the code to set up matrices here

	//######################################################################
	// TODO: 
	// you might want to set the lighting up differently. if you do, 
	// we need to set up the lights AFTER setting up the projection
	//######################################################################

	// enable the lighting
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// top view only needs one light
	if (this->camera == 1) {
		glDisable(GL_LIGHT1);
		glDisable(GL_LIGHT2);
	}
	else {
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
	}

	//*********************************************************************
	//
	// * set the light parameters
	//
	//**********************************************************************
	GLfloat lightPosition1[] = { 0,1,1,0 }; // {50, 200.0, 50, 1.0};
	GLfloat lightPosition2[] = { 1, 0, 0, 0 };
	GLfloat lightPosition3[] = { 0, -1, 0, 0 };
	GLfloat yellowLight[] = { 0.5f, 0.5f, .1f, 1.0 };
	GLfloat whiteLight[] = { 1.0f, 1.0f, 1.0f, 1.0 };
	GLfloat blueLight[] = { .1f,.1f,.3f,1.0 };
	GLfloat grayLight[] = { .3f, .3f, .3f, 1.0 };

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteLight);
	glLightfv(GL_LIGHT0, GL_AMBIENT, grayLight);

	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, yellowLight);

	glLightfv(GL_LIGHT2, GL_POSITION, lightPosition3);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, blueLight);



	//*********************************************************************
	// now draw the ground plane
	//*********************************************************************
	//Get modelview matrix
	glGetFloatv(GL_MODELVIEW_MATRIX, ModelViewMatrex);
	//Get projection matrix
	glGetFloatv(GL_PROJECTION_MATRIX, ProjectionMatrex);
	GLfloat MV[4][4];
	GLfloat P[4][4];
	DimensionTransformation(ModelViewMatrex, MV);
	DimensionTransformation(ProjectionMatrex, P);
	sky->paintSkybox(P, MV);

	setupFloor();
	glDisable(GL_LIGHTING);
	// water->Paint(P, MV);
	// drawFloor(200, 10);

	// Call triangle's render function, pass ModelViewMatrex and ProjectionMatrex
	// triangle->Paint(ProjectionMatrex, ModelViewMatrex);


	//*********************************************************************
	// now draw the object and we need to do it twice
	// once for real, and then once for shadows
	//*********************************************************************
	glEnable(GL_LIGHTING);
	setupObjects();
	drawStuff();


	// this time drawing is for shadows (except for top view)
	if (this->camera != 1) {
		setupShadows();
		drawStuff(true);
		unsetupShadows();
	}

	

	//we manage textures by Trainview class, so we modify square's render function
	square->Begin();
	//Active Texture
	glActiveTexture(GL_TEXTURE0);
	//Bind square's texture
	Textures[0]->bind();
	//pass texture to shader
	square->shaderProgram->setUniformValue("Texture", 0);
	//Call square's render function, pass ModelViewMatrex and ProjectionMatrex
	square->Paint(ProjectionMatrex, ModelViewMatrex);
	square->End();

	// Particle 特效
	// this->ProcessParticles();
	this->DrawParticles();


}

//************************************************************************
//
// * This sets up both the Projection and the ModelView matrices
//   HOWEVER: it doesn't clear the projection first (the caller handles
//   that) - its important for picking
//========================================================================
void TrainView::
setProjection()
//========================================================================
{
	// Compute the aspect ratio (we'll need it)
	float aspect = static_cast<float>(width()) / static_cast<float>(height());

	// Check whether we use the world camp
	if (this->camera == 0) {
		arcball.setProjection(false);
		update();
		// Or we use the top cam
	}
	else if (this->camera == 1) {
		float wi, he;
		if (aspect >= 1) {
			wi = 110;
			he = wi / aspect;
		}
		else {
			he = 110;
			wi = he * aspect;
		}

		// Set up the top camera drop mode to be orthogonal and set
		// up proper projection matrix
		glMatrixMode(GL_PROJECTION);
		glOrtho(-wi, wi, -he, he, -1000, +1000);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(90, 1, 0, 0);
		update();
	}
	// Or do the train view or other view here
	//####################################################################
	// TODO: 
	// put code for train view projection here!	
	//####################################################################
	else {
		glMatrixMode(GL_PROJECTION);
		gluPerspective(120, aspect, 1, 2000);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		int curveIndex = this->m_pTrack->curveIndex;
		int pointIndex = this->m_pTrack->pointIndex;
		Pnt3f eye = this->m_pTrack->samplePoints[curveIndex][pointIndex];
		Pnt3f normal = this->m_pTrack->normalVectors[curveIndex][pointIndex];
		Pnt3f tangent = this->m_pTrack->samplePoints[curveIndex][pointIndex];
		if (pointIndex == this->m_pTrack->samplePoints.front().size() - 1u)
		{
			pointIndex = 0;
			curveIndex = (curveIndex + 1) % this->m_pTrack->samplePoints.size();
		}
		else
		{
			pointIndex++;
		}
		tangent = this->m_pTrack->samplePoints[curveIndex][pointIndex] - tangent;
		tangent = tangent + eye;
		normal.normalize();


		gluLookAt(eye.x, eye.y + 10, eye.z,
			tangent.x, tangent.y + 10, tangent.z,
			normal.x, normal.y, normal.z
		);
#ifdef EXAMPLE_SOLUTION
		trainCamView(this, aspect);
#endif
		update();
	}
}

//************************************************************************
//
// * this draws all of the stuff in the world
//
//	NOTE: if you're drawing shadows, DO NOT set colors (otherwise, you get 
//       colored shadows). this gets called twice per draw 
//       -- once for the objects, once for the shadows
//########################################################################
// TODO: 
// if you have other objects in the world, make sure to draw them
//########################################################################
//========================================================================
void TrainView::drawStuff(bool doingShadows)
{
	//Get modelview matrix
	glGetFloatv(GL_MODELVIEW_MATRIX, ModelViewMatrex);
	//Get projection matrix
	glGetFloatv(GL_PROJECTION_MATRIX, ProjectionMatrex);
	GLfloat MV[4][4];
	GLfloat P[4][4];
	DimensionTransformation(ModelViewMatrex, MV);
	DimensionTransformation(ProjectionMatrex, P);

	// Draw the control points
	// don't draw the control points if you're driving 
	// (otherwise you get sea-sick as you drive through them)
	if (this->camera != 2) {
		for (size_t i = 0; i < this->m_pTrack->points.size(); ++i) {
			if (!doingShadows) {
				if (((int)i) != selectedCube)
					glColor3ub(240, 60, 60);
				else
					glColor3ub(240, 240, 30);
			}
			this->m_pTrack->points[i].draw();
		}
		update();
	}
	// draw the track
	//####################################################################
	// TODO: 
	// call your own track drawing code
	//####################################################################

	this->drawTrack(doingShadows);



#ifdef EXAMPLE_SOLUTION
	drawTrack(this, doingShadows);
#endif

	// draw the train
	//####################################################################
	// TODO: 
	//	call your own train drawing code
	//####################################################################
	AppMain::getInstance()->advanceTrain();
	this->drawTrain(t_time);

	static float d = 0, r = 400;
	static Point3d rotation(0, 0, 0);
	d > 360.f ? d = 0.f : d += 0.1;

	//Test model render
	rotation.y = d;
	/*for (int i = 0; i < 5; i++)
	{
		earth[i]->render(P, MV, false, false);

		spaceTest[i]->updateRotation(rotation, Point3d(earth[i]->getPosition(0) + 20 + (r + 200) *15* cos(d *  PI / 180.0) / 10.f, i * 1000, -(r + 100) * 15* sin(d * PI / 180.0) / 10.f + 80.f));
		spaceTest[i]->setEyePosition(arcball.eyeX, arcball.eyeY, arcball.eyeZ);
		spaceTest[i]->render(P, MV, false, false);
	}
	for (int i = 6; i < 10; i++)
	{
		earth[i]->render(P, MV, false, false);

		spaceTest[i]->updateRotation(rotation, Point3d(earth[i]->getPosition(0) + (r + 100) * 15* cos(d *  PI / 180.0) / 10.f, (i - 5) * 1000, -(r + 100) * 15* sin(d * PI / 180.0) / 10.f - 80.f));
		spaceTest[i]->setEyePosition(arcball.eyeX, arcball.eyeY, arcball.eyeZ);
		spaceTest[i]->render(P, MV, false, false);
	}*/

	//Test shader
	rotation.y = d - 90;
	rotation.z = 0;
	spaceShip->updateRotation(rotation, Point3d(r * cos(d *  PI / 180.0) / 10.f, 10.f, -r * sin(d * PI / 180.0) / 10.f));
	spaceShip->render(P, MV, false, false);
	/*
	rotation.z = 16.69929;
	spaceShipReflection->updateRotation(rotation, Point3d(r * cos(d *  PI / 180.0) / 10.f, 20.f, -r * sin(d * PI / 180.0) / 10.f));
	spaceShipReflection->setEyePosition(arcball.eyeX, arcball.eyeY, arcball.eyeZ);
	spaceShipReflection->render(P, MV, false, false);

	spaceShipRefraction->updateRotation(rotation, Point3d(r * cos(d *  PI / 180.0) / 10.f, 30.f, -r * sin(d * PI / 180.0) / 10.f));
	spaceShipRefraction->setEyePosition(arcball.eyeX, arcball.eyeY, arcball.eyeZ);
	spaceShipRefraction->render(P, MV, false, false);*/
	


#ifdef EXAMPLE_SOLUTION
	// don't draw the train if you're looking out the front window
	if (!tw->trainCam->value())
		drawTrain(this, doingShadows);
#endif
}

void TrainView::
drawTrack(bool doingShadows)
{
	this->m_pTrack->computeSamplePoint();
	if (!doingShadows) {
		glColor3ub(255, 255, 255);
	}
	Pnt3f previous_sample = this->m_pTrack->samplePoints.front().front();
	for (int i = 0; i < this->m_pTrack->samplePoints.size(); i++)
	{
		vector<Pnt3f> &curveSamplePoint = this->m_pTrack->samplePoints[i];
		vector<Pnt3f> &curveNormalVectors = this->m_pTrack->normalVectors[i];
		for (int j = 0; j < curveSamplePoint.size() - 1u; j++)
		{
			Pnt3f sample1 = curveSamplePoint[j];
			Pnt3f sample2 = curveSamplePoint[j + 1];
			Pnt3f normal = curveNormalVectors[j];
			if (j == curveSamplePoint.size() - 2u)
			{
				int breakpoint = 0;
			}
			switch (this->track)
			{
			case 0:
			{
				glBegin(GL_LINES);
				glVertex3f(sample1.x, sample1.y, sample1.z);
				glVertex3f(sample2.x, sample2.y, sample2.z);
				glEnd();
				break;
			}
			case 1:
			{
				Pnt3f tangent = sample2 - sample1;
				Pnt3f cross_t = tangent * normal;
				cross_t.normalize();
				cross_t = cross_t * 2.5f;

				// 畫兩側鐵軌
				glLineWidth(3);
				glBegin(GL_LINES);
				glVertex3f(sample1.x + cross_t.x, sample1.y + cross_t.y, sample1.z + cross_t.z);
				glVertex3f(sample2.x + cross_t.x, sample2.y + cross_t.y, sample2.z + cross_t.z);

				glVertex3f(sample1.x - cross_t.x, sample1.y - cross_t.y, sample1.z - cross_t.z);
				glVertex3f(sample2.x - cross_t.x, sample2.y - cross_t.y, sample2.z - cross_t.z);
				glEnd();
				glLineWidth(1);
				
				float dist = distance(previous_sample, sample1);
				if (dist > RAIL_WIDTH * 2)
				{
					Pnt3f tangentP = (sample1 - sample2);
					tangentP.normalize();
					tangentP = sample1 + tangentP * RAIL_WIDTH;
					
					glBegin(GL_POLYGON);
					glVertex3f(sample1.x + cross_t.x, sample1.y + cross_t.y, sample1.z + cross_t.z);
					glVertex3f(tangentP.x + cross_t.x, tangentP.y + cross_t.y, tangentP.z + cross_t.z);

					glVertex3f(tangentP.x - cross_t.x, tangentP.y - cross_t.y, tangentP.z - cross_t.z);
					glVertex3f(sample1.x - cross_t.x, sample1.y - cross_t.y, sample1.z - cross_t.z);
					glEnd();

					previous_sample = sample1;
				}
				break;
			}
			
			case 3:
			default:
				break;
			}
		}
	}
}

void TrainView::
drawTrain(float t)
{
	

	
	GLfloat MV[4][4];
	GLfloat P[4][4];
	DimensionTransformation(ModelViewMatrex, MV);
	DimensionTransformation(ProjectionMatrex, P);

	
	for (TrainItem item: trainList)
	{
		int curveIndex = item.curveIndex;
		int pointIndex = item.pointIndex;
		Pnt3f _normal = this->m_pTrack->normalVectors[curveIndex][pointIndex];
		Pnt3f _sample1 = this->m_pTrack->samplePoints[curveIndex][pointIndex];
		Pnt3f _sample2;
		if (pointIndex == (int)this->m_pTrack->samplePoints[curveIndex].size() - 1)
			// 這裡有bug, 因為第一條曲線的最後一個點跟第二條曲線的第一個點一樣, 所以計算出的tangent=0,0,0
			_sample2 = this->m_pTrack->samplePoints[(curveIndex + 1) % this->m_pTrack->samplePoints.size()][1];
		else
			_sample2 = this->m_pTrack->samplePoints[curveIndex][pointIndex + 1];

		Point3d normal = Point3d(_normal.x, _normal.y, _normal.z);
		Point3d sample1 = Point3d(_sample1.x, _sample1.y, _sample1.z);
		Point3d sample2 = Point3d(_sample2.x, _sample2.y, _sample2.z);
		Point3d tangent = sample2 - sample1;


		item.train->setEyePosition(-arcball.eyeX, -arcball.eyeY, -arcball.eyeZ);
		item.train->updateRotation(sample1, tangent, normal);
		item.train->render(P, MV);
	}
}

void TrainView::
doPick(int mx, int my)
//========================================================================
{
	// since we'll need to do some GL stuff so we make this window as 
	// active window
	makeCurrent();

	// get the viewport - most reliable way to turn mouse coords into GL coords
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPickMatrix((double)mx, (double)(viewport[3] - my),
		5, 5, viewport);

	// now set up the projection
	setProjection();

	// now draw the objects - but really only see what we hit
	GLuint buf[100];
	glSelectBuffer(100, buf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);


	// draw the cubes, loading the names as we go
	for (size_t i = 0; i < m_pTrack->points.size(); ++i) {
		glLoadName((GLuint)(i + 1));
		m_pTrack->points[i].draw();
	}

	// go back to drawing mode, and see how picking did
	int hits = glRenderMode(GL_RENDER);
	if (hits) {
		// warning; this just grabs the first object hit - if there
		// are multiple objects, you really want to pick the closest
		// one - see the OpenGL manual 
		// remember: we load names that are one more than the index
		selectedCube = buf[3] - 1;
	}
	else // nothing hit, nothing selected
		selectedCube = -1;
}

void TrainView::
initializeMedia()
{
	QMediaPlayer *player;
	player = new QMediaPlayer;
	player->setMedia(QUrl(QUrl::fromLocalFile("D:\\GitHub\\Monopoly\\Monopoly\\music\\Maple Valley.wav")));
	player->setVolume(50);
	player->play();

}

void TrainView::
DrawParticles() {
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTranslatef(0, 0, -60);
	pParticle par;
	par = this->Particles;
	while (par)
	{
		glColor4f(par->r, par->g, par->b, par->life);
		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2d(1, 1);
		glVertex3f(par->xpos + par->size, par->ypos + par->size, par->zpos);
		glTexCoord2d(0, 1);
		glVertex3f(par->xpos - par->size, par->ypos + par->size, par->zpos);
		glTexCoord2d(1, 0);
		glVertex3f(par->xpos + par->size, par->ypos - par->size, par->zpos);
		glTexCoord2d(0, 0);
		glVertex3f(par->xpos - par->size, par->ypos - par->size, par->zpos);
		glEnd();
		par = par->pNext;
	}
}

void TrainView::
AddParticle(Particle ex)
{
	pParticle p;
	p = new Particle;//new particle   
	p->pNext = NULL;
	p->pPrev = NULL;
	p->b = ex.b;
	p->g = ex.g;
	p->r = ex.r;

	p->fade = ex.fade;
	p->life = ex.life;
	p->size = ex.size;
	p->xpos = ex.xpos;
	p->ypos = ex.ypos;
	p->zpos = ex.zpos;
	p->xspeed = ex.xspeed;
	p->yspeed = ex.yspeed;
	p->zspeed = ex.zspeed;
	p->AddCount = ex.AddCount;
	p->AddSpeed = ex.AddSpeed;
	p->bAddParts = ex.bAddParts;
	p->bFire = ex.bFire;
	p->nExpl = ex.nExpl;

	if (!Particles)//當目前的Particle列表為空時
	{
		Particles = p;
		return;
	}
	else
	{
		Particles->pPrev = p;//插入Particle   
		p->pNext = Particles;
		Particles = p;
	}

}

void TrainView::
DeleteAll(pParticle* Part)
{
	while ((*Part))
		DeleteParticle(Part);
}

void TrainView::
DeleteParticle(pParticle* p)
{
	if (!(*p))//假如Particle列表為空
		return;
	if (!(*p)->pNext && !(*p)->pPrev)//假如只有一個Particle，直接删除  
	{
		delete (*p);
		*p = NULL;
		return;
	}
	pParticle tmp;
	if (!(*p)->pPrev)//假如是首節點  
	{
		tmp = (*p);
		*p = (*p)->pNext;
		Particles = *p;
		(*p)->pPrev = NULL;
		delete tmp;
		return;
	}
	if (!(*p)->pNext)//末節點  
	{
		(*p)->pPrev->pNext = NULL;
		delete (*p);
		*p = NULL;
		return;
	}
	else
	{
		//一般情况  
		tmp = (*p);
		(*p)->pPrev->pNext = (*p)->pNext;
		(*p)->pNext->pPrev = (*p)->pPrev;
		*p = (*p)->pNext;
		delete tmp;
	}
}

void TrainView::
InitParticle(Particle& ep)
{
	ep.b = float(rand() % 100) / 60.0f;//顏色隨機
	ep.g = float(rand() % 100) / 60.0f;
	ep.r = float(rand() % 100) / 60.0f;
	ep.life = 1.0f;//初始壽命
	ep.fade = 0.005f + float(rand() % 21) / 10000.0f;//衰减速度
	ep.size = 1;//大小  
	ep.xpos = 30.0f - float(rand() % 601) / 10.0f;//位置 
	ep.ypos = 0.f;
	ep.zpos = 120.f - float(rand() % 401) / 10.0f;
	if (!int(ep.xpos))//x方向速度(z方向相同)
	{
		ep.xspeed = 0.0f;
	}
	else
	{
		if (ep.xpos < 0)
		{
			ep.xspeed = (rand() % int(-ep.xpos)) / 1500.0f;
		}
		else
		{
			ep.xspeed = -(rand() % int(-ep.xpos)) / 1500.0f;
		}
	}
	ep.yspeed = 0.04f + float(rand() % 11) / 1000.0f;//y方向速度(向上)
	ep.bFire = 1;
	ep.nExpl = 1 + rand() % 6;//粒子效果  
	ep.bAddParts = 1;//設定有尾巴 
	ep.AddCount = 0.0f;
	ep.AddSpeed = 0.2f;
	nOfFires++;//粒子數+1 
	AddParticle(ep);//加入粒子列表    
}

void TrainView::
ProcessParticles()
{
	Tick1 = Tick2;
	Tick2 = GetTickCount();
	DTick = float(Tick2 - Tick1);
	DTick *= 0.5f;
	Particle ep;
	if (nOfFires < MAX_FIRES)
	{
		InitParticle(ep);
	}
	pParticle par;
	par = Particles;
	while (par) {
		par->life -= par->fade*(float(DTick)*0.1f);//Particle壽命衰減 
		if (par->life <= 0.05f) {//當壽命小於一定值          
			if (par->nExpl) {//爆炸效果
				switch (par->nExpl) {
				case 1:
					Explosion1(par);
					break;
				case 2:
					Explosion2(par);
					break;
				case 3:
					Explosion3(par);
					break;
				default:
					break;
				}
			}
			if (par->bFire)
				nOfFires--;
			DeleteParticle(&par);
		}
		else
		{
			par->xpos += par->xspeed*DTick;
			par->ypos += par->yspeed*DTick;
			par->zpos += par->zspeed*DTick;
			par->yspeed -= grav * DTick;
			if (par->bAddParts) {//假如有尾巴  
				par->AddCount += 0.01f*DTick;//AddCount變化愈慢，尾巴粒子愈小  
				if (par->AddCount > par->AddSpeed) {//AddSpeed愈大，尾巴粒子愈小  
					par->AddCount = 0;
					ep.b = par->b;
					ep.g = par->g;
					ep.r = par->r;
					ep.life = par->life*0.9f;//壽命變短  
					ep.fade = par->fade*7.0f;//衰减快一些  
					ep.size = 0.6f;//粒子尺寸小一些  
					ep.xpos = par->xpos;
					ep.ypos = par->ypos;
					ep.zpos = par->zpos;
					ep.xspeed = 0.0f;
					ep.yspeed = 0.0f;
					ep.zspeed = 0.0f;
					ep.bFire = 0;
					ep.nExpl = 0;
					ep.bAddParts = 0;//尾巴粒子没有尾巴  
					ep.AddCount = 0.0f;
					ep.AddSpeed = 0.0f;
					AddParticle(ep);
				}
			}
			par = par->pNext;//更新下一粒子    
		}
	}
}

// ****************************************
// ******   Particle 爆炸效果     *********
// ****************************************
void TrainView::
Explosion1(Particle* par)
{
	Particle ep;
	for (int i = 0; i < 100; i++)
	{
		ep.b = float(rand() % 100) / 60.0f;
		ep.g = float(rand() % 100) / 60.0f;
		ep.r = float(rand() % 100) / 60.0f;
		ep.life = 1.0f;
		ep.fade = 0.01f + float(rand() % 31) / 10000.0f;
		ep.size = 0.8f;
		ep.xpos = par->xpos;
		ep.ypos = par->ypos;
		ep.zpos = par->zpos;
		ep.xspeed = 0.02f - float(rand() % 41) / 1000.0f;
		ep.yspeed = 0.02f - float(rand() % 41) / 1000.0f;
		ep.zspeed = 0.02f - float(rand() % 41) / 1000.0f;
		ep.bFire = 0;
		ep.nExpl = 0;
		ep.bAddParts = 0;
		ep.AddCount = 0.0f;
		ep.AddSpeed = 0.0f;
		AddParticle(ep);
	}
}

void TrainView::
Explosion2(Particle* par)
{
	Particle ep;
	for (int i = 0; i < 1000; i++)
	{
		ep.b = par->b;
		ep.g = par->g;
		ep.r = par->r;
		ep.life = 1.0f;
		ep.fade = 0.01f + float(rand() % 31) / 10000.0f;
		ep.size = 0.8f;
		ep.xpos = par->xpos;
		ep.ypos = par->ypos;
		ep.zpos = par->zpos;
		ep.xspeed = 0.02f - float(rand() % 41) / 1000.0f;
		ep.yspeed = 0.02f - float(rand() % 41) / 1000.0f;
		ep.zspeed = 0.02f - float(rand() % 41) / 1000.0f;
		ep.bFire = 0;
		ep.nExpl = 0;
		ep.bAddParts = 0;
		ep.AddCount = 0.0f;
		ep.AddSpeed = 0.0f;
		AddParticle(ep);
	}
}

void TrainView::
Explosion3(Particle* par)
{
	Particle ep;
	float PIAsp = 3.1415926 / 180;
	for (int i = 0; i < 30; i++) {
		float angle = float(rand() % 360)*PIAsp;
		ep.b = par->b;
		ep.g = par->g;
		ep.r = par->r;
		ep.life = 1.5f;
		ep.fade = 0.01f + float(rand() % 31) / 10000.0f;
		ep.size = 0.8f;
		ep.xpos = par->xpos;
		ep.ypos = par->ypos;
		ep.zpos = par->zpos;
		ep.xspeed = (float)sin(angle)*0.01f;
		ep.yspeed = 0.01f + float(rand() % 11) / 1000.0f;
		ep.zspeed = (float)cos(angle)*0.01f;
		ep.bFire = 0;
		ep.nExpl = 0;
		ep.bAddParts = 1;
		ep.AddCount = 0.0f;
		ep.AddSpeed = 0.2f;
		AddParticle(ep);
	}
}

void TrainView::printFPS()
{
	static float framesPerSecond = 0.0f;
	static int fps;
	static float lastTime = 0.0f;
	float currentTime = GetTickCount() * 0.001f;
	++framesPerSecond;
	//printf("FPS: %d\n\n", fps);
	if (currentTime - lastTime > 1.0f)
	{
		lastTime = currentTime;
		fps = (int)framesPerSecond;
		framesPerSecond = 0;
	}
}


void DimensionTransformation(GLfloat source[], GLfloat target[][4])
{
	//for uniform value, transfer 1 dimension to 2 dimension
	int i = 0;
	for (int j = 0; j < 4; j++)
		for (int k = 0; k < 4; k++)
		{
			target[j][k] = source[i];
			i++;
		}
}