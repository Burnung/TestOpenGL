#ifndef GL_KDTREE_H
#define GL_KDTREE_H
#include"GL_BaseType.h"
#include<glm\vec3.hpp>
#include<math.h>
#include<vector>
using namespace std;

const int MAX_DEPTH = 25;    //kdTree��������
const int MIN_COUNT = 10;    //Ҷ�ӽڵ������ε���С����

class GL_kdTree{
	enum AXIS{
		X_AXIS,
		Y_AXIS,
		Z_AXIS
	};
public:
	GL_kdTree();
	~GL_kdTree();
	static GL_kdTree* Build(std::vector<Triangle*>&triangles, int depth);

	bool IsLeaf;
	GL_kdTree *m_PLeftChild;
	GL_kdTree *m_PRightChild;

	vector<Triangle*> m_Triangles;
	AABB_Box m_AABB;     //�ýڵ�İ�Χ�� 

	GL_Material *m_Mat;

	int getTriSize();

	int getAxis(vector<Triangle*> tmpTri);
	int getAxis(AABB_Box &tmpAABB,float &Midp);
	bool InterSect(GL_Ray& ray,GL_ObjIntersection &infersections, float &tmin);
};











#endif