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


	//Initialize texture 
	initializeTexture();
	//Initialize music
	initializeMedia();
	//Initialize skybox
	// initializeSkybox();

	//particle參數
	grav = 0.00003f;
	nOfFires = 0;
	Tick1 = Tick2 = GetTickCount();
	Particles = NULL;

	// 軌道參數
	curve = 0;          // 軌道類別
	DIVIDE_LINE = 80;
	RAIL_WIDTH = 3.f;
	// 其他物件
	spaceShip = new Model(QStringLiteral(":/Object/Resources/Object/Transport_Shuttle_obj.obj"), 20, Point3d(-6.f, 10.f, 3.f));
	//tmp = new Model(QStringLiteral(":/Object/Resources/object/arrow.obj"), 100, Point3d(0.f, 0.f, 3.f));

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

void TrainView::initializeSkybox()
{

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


	setupFloor();
	glDisable(GL_LIGHTING);
	water->Paint(P, MV);
	sky->paintSkybox(P, MV);
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
		glOrtho(-wi, wi, -he, he, 200, -200);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(-90, 1, 0, 0);
		update();
	}
	// Or do the train view or other view here
	//####################################################################
	// TODO: 
	// put code for train view projection here!	
	//####################################################################
	else {
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
	spline_t type_spline = (spline_t)curve;

	switch (type_spline)
	{
	case spline_Linear:
	{
		for (size_t i = 0; i < m_pTrack->points.size(); ++i)
		{
			Pnt3f cp_pos_p1 = m_pTrack->points[i].pos;
			Pnt3f cp_pos_p2 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].pos;
			// orient
			Pnt3f cp_orient_p1 = m_pTrack->points[i].orient;
			Pnt3f cp_orient_p2 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].orient;



			float percent = 1.0f / DIVIDE_LINE;
			float t = 0;
			Pnt3f qt, qt0, qt1, qt2, orient_t, cross_t, previous_qt;

			// 用控制點求出內差點
			qt = (1 - t) * cp_pos_p1 + t * cp_pos_p2;

			previous_qt = qt;
			for (size_t j = 0; j < DIVIDE_LINE; j++) {
				// 處理軌道線條
				qt0 = qt;
				orient_t = (1 - t) * cp_orient_p1 + t * cp_orient_p2;

				t += percent;

				qt = (1 - t) * cp_pos_p1 + t * cp_pos_p2;
				qt1 = qt;

				// 向左右延伸距離
				orient_t.normalize();
				cross_t = (qt1 - qt0) * orient_t;
				cross_t.normalize();
				cross_t = cross_t * 2.5f;

				// 畫出兩側軌道
				glLineWidth(3);

				if (!doingShadows) {
					glColor3ub(32, 32, 64);
				}
				glBegin(GL_LINES);
				glVertex3f(qt0.x + cross_t.x, qt0.y + cross_t.y, qt0.z + cross_t.z);
				glVertex3f(qt1.x + cross_t.x, qt1.y + cross_t.y, qt1.z + cross_t.z);

				glVertex3f(qt1.x - cross_t.x, qt1.y - cross_t.y, qt1.z - cross_t.z);
				glVertex3f(qt0.x - cross_t.x, qt0.y - cross_t.y, qt0.z - cross_t.z);
				glEnd();

				// 畫出鐵軌板子
				float dist = distance(previous_qt, qt0);
				if (dist > RAIL_WIDTH * 2.f)
				{
					Pnt3f tangentP = (qt1 - qt0);
					tangentP.normalize();
					tangentP = qt0 + tangentP * RAIL_WIDTH;
					if (!doingShadows) {
						glColor3ub(255, 255, 255);
					}
					glBegin(GL_POLYGON);
					glVertex3f(qt0.x + cross_t.x, qt0.y + cross_t.y, qt0.z + cross_t.z);
					glVertex3f(tangentP.x + cross_t.x, tangentP.y + cross_t.y, tangentP.z + cross_t.z);

					glVertex3f(tangentP.x - cross_t.x, tangentP.y - cross_t.y, tangentP.z - cross_t.z);
					glVertex3f(qt0.x - cross_t.x, qt0.y - cross_t.y, qt0.z - cross_t.z);
					glEnd();

					previous_qt = qt0;
				}
				glLineWidth(1);
			}
		}
		break;
	}
	default:
	{
		const vector<vector<float>> &M = M_curve[this->curve];
		for (size_t i = 0; i < m_pTrack->points.size(); ++i)
		{
			// position
			const Pnt3f &cp_pos_p1 = m_pTrack->points[i].pos;
			const Pnt3f &cp_pos_p2 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].pos;
			const Pnt3f &cp_pos_p3 = m_pTrack->points[(i + 2) % m_pTrack->points.size()].pos;
			const Pnt3f &cp_pos_p4 = m_pTrack->points[(i + 3) % m_pTrack->points.size()].pos;
			// orient
			const Pnt3f &cp_orient_p1 = m_pTrack->points[i].orient;
			const Pnt3f &cp_orient_p2 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].orient;
			const Pnt3f &cp_orient_p3 = m_pTrack->points[(i + 2) % m_pTrack->points.size()].orient;
			const Pnt3f &cp_orient_p4 = m_pTrack->points[(i + 3) % m_pTrack->points.size()].orient;

			const vector<vector<float>>
				G = {
					{cp_pos_p1.x,cp_pos_p2.x,cp_pos_p3.x,cp_pos_p4.x},
					{cp_pos_p1.y,cp_pos_p2.y,cp_pos_p3.y,cp_pos_p4.y},
					{cp_pos_p1.z,cp_pos_p2.z,cp_pos_p3.z,cp_pos_p4.z},
			},
			G_orient = {
				{cp_orient_p1.x,cp_orient_p2.x,cp_orient_p3.x,cp_orient_p4.x},
				{cp_orient_p1.y,cp_orient_p2.y,cp_orient_p3.y,cp_orient_p4.y},
				{cp_orient_p1.z,cp_orient_p2.z,cp_orient_p3.z,cp_orient_p4.z},
			};

			float percent = 1.0f / (DIVIDE_LINE);
			float t = 0;
			Pnt3f qt, qt0, qt1, qt2, orient_t, cross_t, previous_qt;

			vector<vector<float>>
				T =
			{
				{0},
				{0},
				{0},
				{1}
			};
			// 用控制點求出內差點
			vector<vector<float>> qt_v = Multiply(G, Multiply(M, T));
			qt = Pnt3f(qt_v[0][0], qt_v[1][0], qt_v[2][0]);


			previous_qt = qt;
			for (size_t j = 0; j < DIVIDE_LINE; j++)
			{
				// 處理軌道線條
				T = {
					{pow(t,3)},
					{pow(t,2)},
					{t},
					{1}
				};
				qt_v = Multiply(G, Multiply(M, T));
				qt0 = Pnt3f(qt_v[0][0], qt_v[1][0], qt_v[2][0]);


				vector<vector<float>> orient_v = Multiply(G_orient, Multiply(M, T));
				orient_t = Pnt3f(orient_v[0][0], orient_v[1][0], orient_v[2][0]);


				t += percent;
				T = {
					{pow(t,3)},
					{pow(t,2)},
					{t},
					{1}
				};
				qt_v = Multiply(G, Multiply(M, T));
				qt1 = Pnt3f(qt_v[0][0], qt_v[1][0], qt_v[2][0]);

				// 向左右延伸距離
				orient_t.normalize();
				cross_t = (qt1 - qt0) * orient_t;
				cross_t.normalize();
				cross_t = cross_t * 2.5f;

				// 畫出兩側軌道
				glLineWidth(3);

				if (!doingShadows) {
					glColor3ub(32, 32, 64);
				}
				glBegin(GL_LINES);
				glVertex3f(qt0.x + cross_t.x, qt0.y + cross_t.y, qt0.z + cross_t.z);
				glVertex3f(qt1.x + cross_t.x, qt1.y + cross_t.y, qt1.z + cross_t.z);

				glVertex3f(qt1.x - cross_t.x, qt1.y - cross_t.y, qt1.z - cross_t.z);
				glVertex3f(qt0.x - cross_t.x, qt0.y - cross_t.y, qt0.z - cross_t.z);
				glEnd();

				// 畫出鐵軌板子
				float dist = distance(previous_qt, qt0);
				if (dist > RAIL_WIDTH * 2.f)
				{
					Pnt3f tangentP = (qt1 - qt0);
					tangentP.normalize();
					tangentP = qt0 + tangentP * RAIL_WIDTH;
					if (!doingShadows) {
						glColor3ub(255, 255, 255);
					}
					glBegin(GL_POLYGON);
					glVertex3f(qt0.x + cross_t.x, qt0.y + cross_t.y, qt0.z + cross_t.z);
					glVertex3f(tangentP.x + cross_t.x, tangentP.y + cross_t.y, tangentP.z + cross_t.z);

					glVertex3f(tangentP.x - cross_t.x, tangentP.y - cross_t.y, tangentP.z - cross_t.z);
					glVertex3f(qt0.x - cross_t.x, qt0.y - cross_t.y, qt0.z - cross_t.z);
					glEnd();

					previous_qt = qt0;
				}
				glLineWidth(1);
			}
		}
		break;
	}
	}



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
	rotation.y = d - 90;
	spaceShip->updateRotation(rotation, Point3d(r * cos(d *  PI / 180.0) / 10.f, 10.f, -r * sin(d * PI / 180.0) / 10.f));
	spaceShip->render(P, MV, false, false);
	//tmp->render(false, false);


#ifdef EXAMPLE_SOLUTION
	// don't draw the train if you're looking out the front window
	if (!tw->trainCam->value())
		drawTrain(this, doingShadows);
#endif
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

void TrainView::
drawTrain(float t)
{
	t *= m_pTrack->points.size();
	size_t i;
	for (i = 0; t > 1; t -= 1)
		i++;

	// position
	const Pnt3f &cp_pos_p1 = m_pTrack->points[i].pos;
	const Pnt3f &cp_pos_p2 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].pos;
	const Pnt3f &cp_pos_p3 = m_pTrack->points[(i + 2) % m_pTrack->points.size()].pos;
	const Pnt3f &cp_pos_p4 = m_pTrack->points[(i + 3) % m_pTrack->points.size()].pos;

	// orient
	const Pnt3f &cp_orient_p1 = m_pTrack->points[i].orient;
	const Pnt3f &cp_orient_p2 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].orient;
	const Pnt3f &cp_orient_p3 = m_pTrack->points[(i + 2) % m_pTrack->points.size()].orient;
	const Pnt3f &cp_orient_p4 = m_pTrack->points[(i + 3) % m_pTrack->points.size()].orient;

	vector<vector<float>> &M = M_curve[this->curve];
	vector<vector<float>> T;
	vector<vector<float>> G = {
		{cp_pos_p1.x,cp_pos_p2.x,cp_pos_p3.x,cp_pos_p4.x},
		{cp_pos_p1.y,cp_pos_p2.y,cp_pos_p3.y,cp_pos_p4.y},
		{cp_pos_p1.z,cp_pos_p2.z,cp_pos_p3.z,cp_pos_p4.z}
	};
	vector<vector<float>> G_orient = {
		{cp_orient_p1.x,cp_orient_p2.x,cp_orient_p3.x,cp_orient_p4.x},
		{cp_orient_p1.y,cp_orient_p2.y,cp_orient_p3.y,cp_orient_p4.y},
		{cp_orient_p1.z,cp_orient_p2.z,cp_orient_p3.z,cp_orient_p4.z}
	}, tmp;


	spline_t type_spline = (spline_t)curve;
	Pnt3f qt, qt0, qt1, orient_t, cross_t;
	switch (type_spline) {
	case spline_Linear:
		// Linear
		qt = (1 - t) * cp_pos_p1 + t * cp_pos_p2;
		orient_t = (1 - t) * cp_orient_p1 + t * cp_orient_p2;
		break;

	case spline_CardinalCubic:
	case spline_CubicB_Spline:
		T = { {pow(t,3)}, {pow(t,2)}, {t}, {1} };
		tmp = Multiply(G_orient, Multiply(M, T));
		orient_t = Pnt3f(tmp[0][0], tmp[1][0], tmp[2][0]);
		tmp = Multiply(G, Multiply(M, T));
		qt = Pnt3f(tmp[0][0], tmp[1][0], tmp[2][0]);
		break;
	}
	qt0 = qt;

	t += 0.01f;
	switch (type_spline) {
	case spline_Linear:
		// Linear
		qt = (1 - t) * cp_pos_p1 + t * cp_pos_p2;
		orient_t = (1 - t) * cp_orient_p1 + t * cp_orient_p2;
		break;

	case spline_CardinalCubic:
	case spline_CubicB_Spline:
		T = { {pow(t,3)}, {pow(t,2)}, {t}, {1} };
		tmp = Multiply(G_orient, Multiply(M, T));
		orient_t = Pnt3f(tmp[0][0], tmp[1][0], tmp[2][0]);
		tmp = Multiply(G, Multiply(M, T));
		qt = Pnt3f(tmp[0][0], tmp[1][0], tmp[2][0]);
		break;
	}
	qt1 = qt;


	orient_t.normalize();
	cross_t = (qt1 - qt0) * orient_t;
	cross_t.normalize();
	cross_t = cross_t * 2.5f;
	orient_t = 5 * orient_t;

	qt1 = qt0 - cross_t;
	qt0 = qt0 + cross_t;


	glBegin(GL_POLYGON);
	glVertex3f(qt0.x, qt0.y, qt0.z);
	glVertex3f(qt0.x + orient_t.x, qt0.y + orient_t.y, qt0.z + orient_t.z);

	glVertex3f(qt1.x + orient_t.x, qt1.y + orient_t.y, qt1.z + orient_t.z);
	glVertex3f(qt1.x, qt1.y, qt1.z);
	glEnd();
}


float distance(const Pnt3f &p1, const Pnt3f &p2)
{
	float x_2 = (p1.x - p2.x) * (p1.x - p2.x);
	float y_2 = (p1.y - p2.y) * (p1.y - p2.y);
	float z_2 = (p1.z - p2.z) * (p1.z - p2.z);
	return sqrt(x_2 + y_2 + z_2);
}
vector<vector<float>> Multiply(const vector<vector<float>> &m1, const vector<vector<float>> &m2)
{
	vector<vector<float>> result(m1.size(), vector<float>(m2[0].size(), 0));
	for (int i = 0; i < result.size(); i++)
	{
		for (int j = 0; j < result[0].size(); j++)
		{
			for (int k = 0; k < m1[0].size(); k++)
			{
				result[i][j] += m1[i][k] * m2[k][j];
			}
		}
	}
	return result;
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