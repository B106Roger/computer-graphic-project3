/************************************************************************
     File:        Track.cpp

     Author:     
                  Michael Gleicher, gleicher@cs.wisc.edu
     Modifier
                  Yu-Chi Lai, yu-chi@cs.wisc.edu
     
     Comment:     Container for the "World"

						This provides a container for all of the "stuff" 
						in the world.

						It could have been all global variables, or it could 
						have just been
						contained in the window. The advantage of doing it 
						this way is that
						we might have multiple windows looking at the same 
						world. But, I don't	think we'll actually do that.

						See the readme for commentary on code style

     Platform:    Visio Studio.Net 2003/2005

*************************************************************************/

#include "Track.H"
const int CTrack::DIVIDE_LINE = 150;
const int CTrack::SAMPLE_NUMBER = 10000;

//****************************************************************************
//
// * Constructor
//============================================================================
CTrack::
CTrack() : trainU(0), dirty(true), curveIndex(0), pointIndex(0)
//============================================================================
{
	resetPoints();
	glm::mat4 spline_Linear(1);
	glm::mat4 spline_CardinalCubic(
		-1, 3,-3, 1, 
		 2,-5, 4,-1, 
		-1, 0, 1, 0,
		 0, 2, 0, 0
	);
	glm::mat4 spline_CubicB_Spline(
		-1, 3, -3, 1,
		3, -6, 3, 0, 
		-3, 0, 3, 0,
		1, 4, 1, 0
	);

	
	splineType = LINE;
	M_curve.push_back(spline_Linear);
	M_curve.push_back(spline_CardinalCubic);
	M_curve.push_back(spline_CubicB_Spline);

	tensionScale = 1.f;
}

//****************************************************************************
//
// * provide a default set of points
//============================================================================
void CTrack::
resetPoints()
//============================================================================
{

	points.clear();
	points.push_back(ControlPoint(Pnt3f(25, 15, 25)));
	points.push_back(ControlPoint(Pnt3f(-25, 15, 25)));
	points.push_back(ControlPoint(Pnt3f(-25, 15, -25)));
	points.push_back(ControlPoint(Pnt3f(25, 15, -25)));

	// we had better put the train back at the start of the track...
	trainU = 0.0;
}

//****************************************************************************
//
// * Handy utility to break a string into a list of words
//============================================================================
void breakString(char* str, std::vector<const char*>& words) 
//============================================================================
{
	// start with no words
	words.clear();

	// scan through the string, starting at the beginning
	char* p = str;

	// stop when we hit the end of the string
	while(*p) {
		// skip over leading whitespace - stop at the first character or end of string
		while (*p && *p<=' ') p++;

		// now we're pointing at the first thing after the spaces
		// make sure its not a comment, and that we're not at the end of the string
		// (that's actually the same thing)
		if (! (*p) || *p == '#')
		break;

		// so we're pointing at a word! add it to the word list
		words.push_back(p);

		// now find the end of the word
		while(*p > ' ') p++;	// stop at space or end of string

		// if its ethe end of the string, we're done
		if (! *p) break;

		// otherwise, turn this space into and end of string (to end the word)
		// and keep going
		*p = 0;
		p++;
	}
}

//****************************************************************************
//
// * The file format is simple
//   first line: an integer with the number of control points
//	  other lines: one line per control point
//   either 3 (X,Y,Z) numbers on the line, or 6 numbers (X,Y,Z, orientation)
//============================================================================
void CTrack::
readPoints(const char* filename)
//============================================================================
{
	FILE* fp = fopen(filename,"r");
	if (!fp) {
		printf("Can't Open File!\n");
	} 
	else {
		char buf[512];

		// first line = number of points
		fgets(buf,512,fp);
		size_t npts = (size_t) atoi(buf);

		if( (npts<4) || (npts>65535)) {
			printf("Illegal Number of Points Specified in File");
		} else {
			points.clear();
			// get lines until EOF or we have enough points
			while( (points.size() < npts) && fgets(buf,512,fp) ) {
				Pnt3f pos,orient;
				vector<const char*> words;
				breakString(buf,words);
				if (words.size() >= 3) {
					pos.x = (float) strtod(words[0],0);
					pos.y = (float) strtod(words[1],0);
					pos.z = (float) strtod(words[2],0);
				} else {
					pos.x=0;
					pos.y=0;
					pos.z=0;
				}
				if (words.size() >= 6) {
					orient.x = (float) strtod(words[3],0);
					orient.y = (float) strtod(words[4],0);
					orient.z = (float) strtod(words[5],0);
				} else {
					orient.x = 0;
					orient.y = 1;
					orient.z = 0;
				}
				orient.normalize();
				points.push_back(ControlPoint(pos,orient));
			}
		}
		fclose(fp);
	}
	trainU = 0;
}

//****************************************************************************
//
// * write the control points to our simple format
//============================================================================
void CTrack::
writePoints(const char* filename)
//============================================================================
{
	FILE* fp = fopen(filename,"w");
	if (!fp) {
		printf("Can't open file for writing");
	} else {
		fprintf(fp,"%ud\n",points.size());
		for(size_t i=0; i<points.size(); ++i)
			fprintf(fp,"%g %g %g %g %g %g\n",
				points[i].pos.x, points[i].pos.y, points[i].pos.z, 
				points[i].orient.x, points[i].orient.y, points[i].orient.z);
		fclose(fp);
	}
}


//****************************************************************************
//
// * recompute point, normal, tangent
//============================================================================
void CTrack::
computeSamplePoint()
{
	if (dirty == false)
		return;
	else
		dirty = false;

	samplePoints.resize(this->points.size());
	normalVectors.resize(this->points.size());
	for (int i = 0; i < (int)this->points.size(); ++i)
	{
		samplePoints[i].resize(DIVIDE_LINE + 1);
		normalVectors[i].resize(DIVIDE_LINE + 1);
	}
	Pnt3f previousPoint(0, 0, 0);

	switch (splineType)
	{
	case LINE:
		
		for (size_t i = 0; i < this->points.size(); ++i)
		{
			Pnt3f cp_pos_p1 = this->points[i].pos;
			Pnt3f cp_pos_p2 = this->points[(i + 1) % this->points.size()].pos;
			// orient
			Pnt3f cp_orient_p1 = this->points[i].orient;
			Pnt3f cp_orient_p2 = this->points[(i + 1) % this->points.size()].orient;

			float percent = 1.0f / DIVIDE_LINE;
			float t = 0;
			Pnt3f qt, qt0, orient_t1, orient_t2, tangent, normal;

			// 用控制點求出內差點
			qt = (1 - t) * cp_pos_p1 + t * cp_pos_p2;

			for (size_t j = 0; j < DIVIDE_LINE; j++) {
				// 取樣點C(t)的pos跟normal
				qt0 = (1 - t) * cp_pos_p1 + t * cp_pos_p2;
				orient_t1 = (1 - t) * cp_orient_p1 + t * cp_orient_p2;
				tangent = qt0 - previousPoint;
				orient_t2 = orient_t1 * tangent;
				normal = tangent * orient_t2;
				normal.normalize();

				samplePoints[i][j] = qt0;
				normalVectors[i][j] = normal;
				t += percent;
				previousPoint = qt0;
			}
		}
		break;
	default:
		float tension = (splineType == 1 ? 1 / 2.f : 1 / 6.f);
		glm::mat4 M = M_curve[splineType] * (tension * tensionScale);
		for (size_t i = 0; i < this->points.size(); ++i)
		{
			// position
			const Pnt3f &cp_pos_p1 = this->points[i].pos;
			const Pnt3f &cp_pos_p2 = this->points[(i + 1) % this->points.size()].pos;
			const Pnt3f &cp_pos_p3 = this->points[(i + 2) % this->points.size()].pos;
			const Pnt3f &cp_pos_p4 = this->points[(i + 3) % this->points.size()].pos;
			// orient
			const Pnt3f &cp_orient_p1 = this->points[i].orient;
			const Pnt3f &cp_orient_p2 = this->points[(i + 1) % this->points.size()].orient;
			const Pnt3f &cp_orient_p3 = this->points[(i + 2) % this->points.size()].orient;
			const Pnt3f &cp_orient_p4 = this->points[(i + 3) % this->points.size()].orient;
			glm::mat4x3 G(
				cp_pos_p1.x, cp_pos_p1.y, cp_pos_p1.z, 
				cp_pos_p2.x, cp_pos_p2.y, cp_pos_p2.z, 
				cp_pos_p3.x, cp_pos_p3.y, cp_pos_p3.z, 
				cp_pos_p4.x, cp_pos_p4.y, cp_pos_p4.z
			);
			glm::mat4x3 G_orient(
				cp_orient_p1.x, cp_orient_p1.y, cp_orient_p1.z,
				cp_orient_p2.x, cp_orient_p2.y, cp_orient_p2.z,
				cp_orient_p3.x, cp_orient_p3.y, cp_orient_p3.z,
				cp_orient_p4.x, cp_orient_p4.y, cp_orient_p4.z
			);

			float percent = 1.0f / (DIVIDE_LINE);
			float t = 0;
			glm::vec3 qt, qt0, orient_t1, orient_t2, tangent, normal;

			glm::vec4 T(0, 0, 0, 1);
			// 用控制點求出內差點
			glm::vec3 qt_v = G * (M *T);
			qt = qt_v;


			previousPoint = Pnt3f(qt.x, qt.y, qt.z);
			for (size_t j = 0; j < DIVIDE_LINE; j++)
			{
				// 處理軌道線條
				T = glm::vec4(pow(t, 3), pow(t, 2), t, 1);
				//qt_v = Multiply(G, Multiply(M, T));
				qt_v = G * (M * T);
				qt0 = qt_v;

				glm::vec3 orient_v = G_orient * (M * T);;
				orient_t1 = orient_v;
				tangent = qt0 - glm::vec3(previousPoint.x, previousPoint.y, previousPoint.z);
				orient_t2 = glm::cross(orient_t1, tangent);
				normal = glm::cross(tangent, orient_t2);
				normal /= sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);

				samplePoints[i][j] = Pnt3f(qt0.x, qt0.y, qt0.z);
				normalVectors[i][j] = Pnt3f(normal.x, normal.y, normal.z);

				t += percent;
			}
			
		}
		break;
	}

	for (int i = 0; i < this->samplePoints.size(); i++)
	{
		samplePoints[i][DIVIDE_LINE] = samplePoints[(i + 1) % this->samplePoints.size()].front();
		normalVectors[i][DIVIDE_LINE] = normalVectors[(i + 1) % this->normalVectors.size()].front();
	}
}

void CTrack::
computeSegmentArclength()
{
	segmentArclength.resize(this->points.size() - 1u);
	int pointSize = (int)this->points.size();

	for (int i = 0; i < segmentArclength.size(); i++)
	{
		switch (splineType)
		{
		case LINE:
		{
			segmentArclength[i] =
				distance(this->points[i % pointSize].pos, this->points[(i + 1) % pointSize].pos);
			break;
		}
		case CARDINAL:
		{
			Pnt3f &P1 = this->points[i % pointSize].pos;
			Pnt3f &P2 = this->points[(i + 1) % pointSize].pos;
			Pnt3f &P3 = this->points[(i + 2) % pointSize].pos;
			Pnt3f &P4 = this->points[(i + 3) % pointSize].pos;

			Pnt3f a = 3 * P2 - 3 * P3 - P1 + P4;
			Pnt3f b = 3 * P1 - 6 * P2 + 3 * P3;
			Pnt3f c = 3 * (P3 - P1);
			Pnt3f d = P1 + 4 * P2 + P3;
		}
		break;
		case B_SPLINE:
			
			break;
		default:
			break;
		}
		;
		this->points[i % pointSize], this->points[(i + 1) % pointSize];
	}
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