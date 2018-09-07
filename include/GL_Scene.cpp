#include"GL_Scene.h"
#include"PHO_PathTracer.h"
GL_DEFINE_SINGLETON(GL_Scene);

GL_Scene::~GL_Scene(){
	clear();
}

void GL_Scene::clear(){
	for (auto item : m_Objects){
		SAFERELEASE(item);
	}
	m_Objects.clear();
}

void GL_Scene::addObject(Object* obj){
	if (obj == nullptr)
		return;
	m_Objects.push_back(obj);
}

bool GL_Scene::addModel(std::string &filename, bool kdTree){
	Model*newModel = new Model;
	if (!newModel){
		char msg[512];
		sprintf_s(msg, "error to create model : %s", filename.c_str());
		ERROROUT(msg);
		exit(1);
	}
	if (!newModel->LoadFromFile(filename, kdTree))
		return false;
	addObject(newModel);
}

bool GL_Scene::addSphereObj(glm::vec3 &center, float raduis,GL_Material &mat){
	SphereObj *tmpSphere = new SphereObj;
	if (tmpSphere == nullptr){
		ERROROUT("error to creat sphere");
		return false;
	}
	tmpSphere->Init(raduis,center,mat);
	addObject(tmpSphere);
		
}

void GL_Scene::Render(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (auto item : m_Objects){
		if (item == nullptr)
			continue;
		item->Render();
	}
}
GL_ObjIntersection GL_Scene::Intersect(GL_Ray &ray){
	GL_ObjIntersection ret_its = GL_ObjIntersection();
	GL_ObjIntersection tmp;
	for (auto item : m_Objects){
		GL_ObjIntersection tmp;
		item->InterSect(ray, tmp, ret_its.m_Dis);
		if (tmp.m_IsHit)// && tmp.m_Dis < ret_its.m_Dis)
			ret_its = tmp;
	}
	return ret_its;
}

glm::vec3 GL_Scene::GoTrace(GL_Ray &ray,int n_depth){

	GL_ObjIntersection myInter =  Intersect(ray);
	if (!myInter.m_IsHit)
		return glm::vec3(0, 0, 0);
	GL_Material *tMat = myInter.m_Material;
	if (tMat->m_MaterialType == EMMI)
		return tMat->m_emission;
	//����˹ת�� ��������������

	glm::vec3 T_Col = tMat->m_colour;
	float Col_Max = std::max(std::max(T_Col.r, T_Col.g), T_Col.b);
	if (n_depth >= PHO_PahtTracer::Instance().GetMaxDepth()){
		if (Col_Max < PHO_Random::Instance().GetDouble())
			return glm::vec3(0, 0, 0);// tMat->m_colour;
		T_Col *= (1.0f / Col_Max);
	}
	//���߿�������ģ���ڲ�����
	glm::vec3 orNormal = myInter.m_Vertex.normal;
	glm::vec3 corNormal = glm::dot(orNormal, ray.m_Dirction) < 0 ? orNormal : -1.0f * orNormal;
	//���������

	if (tMat->m_MaterialType == MaterialType::DIFF){  //Ϊ��������� ������ɹ���

		float Theta = PHO_Random::Instance().GetDouble() * PI *2;
		float Tlen2 = PHO_Random::Instance().GetDouble();
		float Tlen = sqrtf(Tlen2);

		glm::vec3 x_axi = glm::normalize(glm::cross(glm::vec3(0, 1, 0), corNormal));
		glm::vec3 y_axi = glm::normalize(glm::cross(corNormal, x_axi));
		glm::vec3 t_dir = x_axi * cos(Theta) * Tlen + y_axi * sin(Theta) * Tlen + corNormal * sqrtf(1 - Tlen2);
		t_dir = glm::normalize(t_dir);
		GL_Ray newRay(myInter.m_Vertex.pos, t_dir);
		return  T_Col * GoTrace(newRay, n_depth + 1);
	}  //����Ǿ��淴��
	if (tMat->m_MaterialType == MaterialType::SPEC){
		//���㷴�����
		glm::vec3 TDir = glm::reflect(-ray.m_Dirction, corNormal);
		GL_Ray new_Ray(myInter.m_Vertex.pos, TDir);
		return  T_Col * GoTrace(new_Ray, n_depth + 1);
	}
	//��ô�������� ���о��淴������͸��

	glm::vec3 refdir = glm::reflect(-ray.m_Dirction, corNormal); //�������
	GL_Ray refRay(myInter.m_Vertex.pos, refdir);
	float Trefra = glm::dot(corNormal, orNormal) > 0 ? 1.0f / tMat->m_Refra : tMat->m_Refra;  //����ʵ��ģ���ڲ�
	float cosTheta = glm::dot(ray.m_Dirction, corNormal);  //��ʵ��-cos
	if (1 - cosTheta * cosTheta > 1.0f/(Trefra * Trefra)){  //����ȫ����

		return tMat->m_colour * GoTrace(refRay, n_depth + 1);
	}
	//�����������
	//return glm::vec3(0, 0, 0);
	glm::vec3 refradir = glm::normalize(glm::refract(ray.m_Dirction, corNormal, 1.0f / Trefra));

	//ʹ�÷�������ʽ���� ����ͷ���Ĺ���
	double f0 = (Trefra - 1)*(Trefra - 1) / ((Trefra + 1) * (Trefra + 1));
	double Pfre = f0 + (1 - f0) * pow(1 + cosTheta, 5);  //����ǿ��
	double pfra = 1.0 - Pfre;                            //����ǿ��
	float Tf = 0.25 + Pfre * 0.5f;     //����˹ת��
	float AllRe = Pfre / Tf;
	float AllFra = pfra / (1 - Tf);
	GL_Ray FraRay(myInter.m_Vertex.pos, refradir);
	if (n_depth <= 1){
		glm::vec3 col_re = (float)Pfre * GoTrace(refRay, n_depth + 1);
		glm::vec3 col_fra = (float)pfra * GoTrace(FraRay, n_depth + 1);
		return  tMat->m_colour *(col_re + col_fra);
	}
	if (PHO_Random::Instance().GetDouble() < Tf)
		return AllRe * tMat->m_colour * GoTrace(refRay, n_depth + 1);
	return AllFra * tMat->m_colour * GoTrace(FraRay, n_depth + 1);


}
void GL_Scene::RegisterMat(GL_Material* tMat){
	std::map<GL_Material*, int>::iterator it = m_MatToIndexMap.find(tMat);
	if (it == m_MatToIndexMap.end()){
		m_MatToIndexMap[tMat] = m_MatVec.size();
		m_MatVec.push_back(tMat);
	}
}

int GL_Scene::GetMatIndex(GL_Material* tMat){
	std::map<GL_Material*, int>::iterator it = m_MatToIndexMap.find(tMat);
	if (it == m_MatToIndexMap.end())
		RegisterMat(tMat);
	return m_MatToIndexMap[tMat];
}

//#include "cuda/Cuda_Scene.cuh"
/*
void GL_Scene::PutSceneToCuda(){
	CUDA_InitCuda();
	CUDA_SetCudaSceneMat(m_MatVec);
	for (auto item : m_Objects){
		if (item->m_Type == Object::MODEL_SPHERE)
			CUDA_AddSphere(dynamic_cast<SphereObj*>(item));
		else if (item->m_Type == Object::MODEL_MESH){
			Model *tmpModel = dynamic_cast<Model*>(item);
			CUDA_AddKdTree(tmpModel->GetTris());
		}

	}
}*/