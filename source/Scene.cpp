#include "Scene.h"
#include "Utils.h"
#include "Material.h"
#include "Timer.h"

namespace dae
{

#pragma region Base Scene
	//Initialize Scene with Default Solid Color Material (RED)
	Scene::Scene() :
		m_Materials({ new Material_SolidColor({1,0,0}) })
	{
		m_SphereGeometries.reserve(32);
		m_PlaneGeometries.reserve(32);
		m_TriangleMeshGeometries.reserve(32);
		m_Lights.reserve(32);
	}

	Scene::~Scene()
	{
		for (auto& pMaterial : m_Materials)
		{
			delete pMaterial;
			pMaterial = nullptr;
		}

		m_Materials.clear();
	}

	void dae::Scene::GetClosestHit(const Ray& viewRay, HitRecord& closestHit) const
	{		
		Ray ray = viewRay;
		// Check the planes
		const size_t planeGeometriesSize{ m_PlaneGeometries.size() };
		for (size_t i{}; i < planeGeometriesSize; ++i)
		{
			GeometryUtils::HitTest_Plane(m_PlaneGeometries[i], ray, closestHit);
			ray.max = closestHit.t;
		}

		// Check the spheres
		const size_t sphereGeometriesSize{ m_SphereGeometries.size() };
		for (size_t i{}; i < sphereGeometriesSize; ++i)
		{
			GeometryUtils::HitTest_Sphere(m_SphereGeometries[i], ray, closestHit);
			ray.max = closestHit.t;
		}		

		// Triangles
		const size_t triangleMeshGeometriesSize{ m_TriangleMeshGeometries.size() };
		for (size_t i{}; i < triangleMeshGeometriesSize; ++i)
		{
			GeometryUtils::HitTest_TriangleMesh(m_TriangleMeshGeometries[i], ray, closestHit, false);
			ray.max = closestHit.t;
		}
		
	}

	bool Scene::DoesHit(const Ray& ray) const
	{
		for (const Plane& plane : m_PlaneGeometries)
		{
			if (GeometryUtils::HitTest_Plane(plane, ray))
				return true;
		}

		for (const Sphere& sphere : m_SphereGeometries)
		{
			if (GeometryUtils::HitTest_Sphere(sphere, ray))
				return true;
		}		

		for (const TriangleMesh& triangleMesh : m_TriangleMeshGeometries)
		{
			if (GeometryUtils::HitTest_TriangleMesh(triangleMesh, ray))
				return true;
		}

		return false;
	}

#pragma region Scene Helpers
	Sphere* Scene::AddSphere(const Vector3& origin, float radius, unsigned char materialIndex)
	{
		Sphere s;
		s.origin = origin;
		s.radius = radius;
		s.materialIndex = materialIndex;

		m_SphereGeometries.emplace_back(s);
		return &m_SphereGeometries.back();
	}

	Plane* Scene::AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex)
	{
		Plane p;
		p.origin = origin;
		p.normal = normal;
		p.materialIndex = materialIndex;

		m_PlaneGeometries.emplace_back(p);
		return &m_PlaneGeometries.back();
	}

	TriangleMesh* Scene::AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex)
	{
		TriangleMesh m{};
		m.cullMode = cullMode;
		m.materialIndex = materialIndex;

		m_TriangleMeshGeometries.emplace_back(m);
		return &m_TriangleMeshGeometries.back();
	}

	Light* Scene::AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color)
	{
		Light l;
		l.origin = origin;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Point;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	Light* Scene::AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color)
	{
		Light l;
		l.direction = direction;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Directional;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	unsigned char Scene::AddMaterial(Material* pMaterial)
	{
		m_Materials.push_back(pMaterial);
		return static_cast<unsigned char>(m_Materials.size() - 1);
	}
#pragma endregion
#pragma endregion

#pragma region SCENE W1
	void Scene_W1::Initialize()
	{
		//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });


		//Spheres
		AddSphere({ -25.f, 0.f, 100.f }, 50.f, matId_Solid_Red);
		AddSphere({ 25.f, 0.f, 100.f }, 50.f, matId_Solid_Blue);

		//Plane
		AddPlane({ -75.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 75.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.f, -75.f, 0.f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 75.f, 0.f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 125.f }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);
	}
#pragma endregion
#pragma region SCENE W2
	void Scene_W2::Update(dae::Timer* pTimer)
	{
		Scene::Update(pTimer);
		++currentColorOffset;
		Material_SolidColor* matChanging{ static_cast<Material_SolidColor*>(m_Materials[matId_Changing_Color]) };

		// Make every sphere shift through colors
		const size_t sphereGeometriesSize{ m_SphereGeometries.size() };
		for (size_t i{}; i < sphereGeometriesSize; ++i)
		{
			Sphere& sphere{ m_SphereGeometries[i] };
			//sphere.materialIndex = static_cast<unsigned char>(i % m_Materials.size());
			const float offSet{ abs(currentColorOffset % 255 + 1 - 128) / 255.0f };
			const float colorRed{ 0.5f + offSet };
			const float colorGreen{ 1.0f - offSet };
			const float colorBlue{ 0.0f };

			ColorRGB color{ colorRed,colorGreen,colorBlue };
			matChanging->SetColor(color);

		}

	}
	void Scene_W2::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.SetFov(45.0f);

		// default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });
		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		matId_Changing_Color = AddMaterial(new Material_SolidColor{ colors::Cyan });

		// Planes
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, matId_Solid_Green);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, matId_Solid_Green);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f }, matId_Solid_Magenta);

		// Spheres
		//AddSphere({ -1.75f, 1.f, 0.f }, .75f, matId_Solid_Red);
		//AddSphere({ 0.f, 1.f, 0.f }, .75f, matId_Solid_Blue);
		//AddSphere({ 1.75f, 1.f, 0.f }, .75f, matId_Solid_Red);
		//AddSphere({ -1.75f, 3.f, 0.f }, .75f, matId_Solid_Blue);
		//AddSphere({ 0.f, 3.f, 0.f }, .75f, matId_Solid_Red);
		//AddSphere({ 1.75f, 3.f, 0.f }, .75f, matId_Solid_Blue);		

		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matId_Changing_Color);
		AddSphere({ 0.f, 1.f, 0.f }, .75f, matId_Changing_Color);
		AddSphere({ 1.75f, 1.f, 0.f }, .75f, matId_Changing_Color);
		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matId_Changing_Color);
		AddSphere({ 0.f, 3.f, 0.f }, .75f, matId_Changing_Color);
		AddSphere({ 1.75f, 3.f, 0.f }, .75f, matId_Changing_Color);

		// Light
		AddPointLight({ 0.f, 5.f, -5.f }, 70.f, colors::White);
	}
#pragma endregion
#pragma region SCENE W3
	void Scene_W3::Initialize()
	{
		m_Camera.origin = { 0.0f, 3.0f, -9.0f };
		m_Camera.SetFov(45.0f);

		const auto matCt_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.96f, 0.915f }, 1.f, 1.f));
		const auto matCt_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.96f, 0.915f }, 1.f, 0.6f));
		const auto matCt_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.96f, 0.915f }, 1.f, 0.1f));

		const auto matCt_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.f, 1.f));
		const auto matCt_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.f, 0.6f));
		const auto matCt_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.f, 0.1f));

		const auto matLamber_GrayBlue = AddMaterial(new Material_Lambert({ 0.49f, 0.57f, 0.57f }, 1.0f));

		// Planes
		AddPlane({ 0.0f, 0.0f, 10.0f }, { 0.0f, 0.0f, -1.0f }, matLamber_GrayBlue);  // BACK
		AddPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, matLamber_GrayBlue);  // BOTTOM
		AddPlane({ 0.0f, 10.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, matLamber_GrayBlue);  // TOP
		AddPlane({ 5.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, matLamber_GrayBlue);  // RIGHT
		AddPlane({ -5.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, matLamber_GrayBlue);  // LEFT

		AddPlane({ 0.0f, 0.0f, -100.0f }, { 0.0f, 0.0f, 1.0f }, matLamber_GrayBlue);  // BEHIND

		//// TEMP Lambert-Phone spheres & materials
		const auto matLambertPhong1 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 3.0f));
		const auto matLambertPhong2 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 15.0f));
		const auto matLambertPhong3 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 50.0f));

		//AddSphere(Vector3(-1.75f, 1.0f, 0.f), 0.75f, matLambertPhong1);
		//AddSphere(Vector3(0.0f, 1.0f, 0.f), 0.75f, matLambertPhong2);
		//AddSphere(Vector3(1.75f, 1.0f, 0.f), 0.75f, matLambertPhong3);

		// Spheres
		AddSphere({ -1.75f, 1.0f, 0.0f }, 0.75f, matCt_GrayRoughMetal);
		AddSphere({ 0.0f, 1.0f, 0.0f }, 0.75f, matCt_GrayMediumMetal);
		AddSphere({ 1.75f, 1.0f, 0.0f }, 0.75f, matCt_GraySmoothMetal);

		AddSphere({ -1.75f, 3.0f, 0.0f }, 0.75f, matCt_GrayRoughPlastic);
		AddSphere({ 0.0f, 3.0f, 0.0f }, 0.75f, matCt_GrayMediumPlastic);
		AddSphere({ 1.75f, 3.0f, 0.0f }, 0.75f, matCt_GraySmoothPlastic);

		// Lights
		AddPointLight(Vector3{ 0.0f, 5.0f, 5.0f }, 50.0f, ColorRGB{ 1.0f, 0.61f, 0.45f });  // BACKLIGHT
		AddPointLight(Vector3{ -2.5f, 5.0f, -5.0f }, 70.0f, ColorRGB{ 1.0f, 0.8f, 0.45f }); // FRONT LIGHT LEFT
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.0f }, 50.0f, ColorRGB{ 0.34f, 0.47f, 0.68f }); // FRONT LIGHT RIGHT

	}
#pragma endregion
	void Scene_W3_Test::Initialize()
	{
		m_Camera.origin = { 0.f, 1.f, -5.0f };
		m_Camera.SetFov(45.0f);

		const auto matLambert_Red = AddMaterial(new Material_Lambert(colors::Red, 1.f));
		const auto matLambert_Blue = AddMaterial(new Material_LambertPhong(colors::Blue, 1.f, 1.f, 60.0f));
		const auto matLambert_Yellow = AddMaterial(new Material_Lambert(colors::Yellow, 1.f));
		const auto matCt_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.960f, 0.915f }, 1.f, 0.1f));

		//// Triangles
		//TriangleCullMode cullMode(TriangleCullMode::NoCulling);

		//TriangleMesh* pTriangle{ AddTriangleMesh(cullMode, matLambert_Blue) };
		//pTriangle->AppendTriangle({
		//	{ -5.0f, 0.0f, 10.0f },
		//	{ 0.0f, 10.0f, 10.0f },
		//	{ 5.0f, 0.0f, 10.0f }}, true);

		AddSphere({ -.75f, 1.f, .0f }, 1.0f, matLambert_Red);
		AddSphere({ .75f, 1.f, .0f }, 1.0f, matLambert_Blue);
		AddSphere({ 2.25f, 1.f, .0f }, 1.0f, matCt_GraySmoothMetal);

		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matLambert_Yellow);

		AddPointLight({ 0.f, 5.f, 5.f }, 25.f, colors::White);

		AddPointLight({ 0.f, 2.5f, -5.f }, 25.f, colors::White);
		AddDirectionalLight(Vector3{ 0.5f, -0.5f, -0.5f }.Normalized(), 50.0f, colors::Red);
	}
	void Scene_W4_TestScene::Initialize()
	{
		m_Camera.origin = { 0.f, 1.f, -5.f };
		m_Camera.SetFov(45.0f);

		// Materials
		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, .57f, .57f }, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(ColorRGB(colors::White), 1.f));

		// Planes
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f }, matLambert_GrayBlue);  // BACK
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matLambert_GrayBlue);  // BOTTOM
		AddPlane({ 0.f, 10.0f, 0.f }, { 0.f, -1.f, 0.f }, matLambert_GrayBlue);  // TOP
		AddPlane({ 5.0f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, matLambert_GrayBlue);  // RIGHT
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, matLambert_GrayBlue);  // LEFT	


		pMesh = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
		pMesh->positions = { {-0.75f, -1.f, 0.f}, {-0.75f, 1.0f, 0.f}, {0.75f, 1.f, 1.f}, {0.75f, -1.f, 0.f} };
		pMesh->indices = {
			0, 1, 2,
			0, 2, 3
		};

		pMesh->CalculateNormals();

		pMesh->Translate({ -0.f, 1.5f, 0.f });
		pMesh->UpdateAABB();
		pMesh->UpdateTransforms();

		//pMesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		//Utils::ParseOBJ("Resources/simple_quad.obj", pMesh->positions, pMesh->normals, pMesh->indices);

		//pMesh->CalculateNormals();
		//pMesh->Translate({ 0.f, 1.f, 0.f });
		//pMesh->Scale({ .7f, .7f, .7f });
		//pMesh->UpdateAABB();
		//pMesh->UpdateTransforms();

		// Lights
		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); // BACKLIGHT
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); // FRONT LIGHT LEFT
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ 0.34f, .47f, .68f });

	}

	void Scene_W4_TestScene::Update(dae::Timer* pTimer)
	{
		Scene::Update(pTimer);  // Base class update

		// Rotate the trianglemesh frame by frame
		// Ptimer gettotal time will increase the longer the scene runs (accumulated time)

		pMesh->RotateY(PI_DIV_4 * pTimer->GetTotal());
		pMesh->UpdateTransforms();

	}

	void Scene_W4_ReferenceScene::Initialize()
	{
		sceneName = "Reference Scene";
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.SetFov(45.0f);

		// Materials
		const auto matCt_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.96f, 0.915f }, 1.f, 1.f));
		const auto matCt_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.96f, 0.915f }, 1.f, 0.6f));
		const auto matCt_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.96f, 0.915f }, 1.f, 0.1f));

		const auto matCt_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.f, 1.f));
		const auto matCt_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.f, 0.6f));
		const auto matCt_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.f, 0.1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ 0.49f, 0.57f, 0.57f }, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

		// Planes
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f }, matLambert_GrayBlue);	// BACK
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matLambert_GrayBlue);	// BOTTOM
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f }, matLambert_GrayBlue);  // TOP
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, matLambert_GrayBlue);	// RIGHT
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, matLambert_GrayBlue);	// LEFT

		// Spheres
		AddSphere({ -1.75f, 1.0f, 0.0f }, 0.75f, matCt_GrayRoughMetal);
		AddSphere({ 0.0f, 1.0f, 0.0f }, 0.75f, matCt_GrayMediumMetal);
		AddSphere({ 1.75f, 1.0f, 0.0f }, 0.75f, matCt_GraySmoothMetal);

		AddSphere({ -1.75f, 3.0f, 0.0f }, 0.75f, matCt_GrayRoughPlastic);
		AddSphere({ 0.0f, 3.0f, 0.0f }, 0.75f, matCt_GrayMediumPlastic);
		AddSphere({ 1.75f, 3.0f, 0.0f }, 0.75f, matCt_GraySmoothPlastic);

		// Triangles
		const Triangle baseTriangle = { { -.75f, 1.5f, 0.f }, { .75f, 0.f, 0.f }, { -.75f, 0.f, 0.f } };

		m_Meshes[0] = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		m_Meshes[0]->AppendTriangle(baseTriangle, true);
		m_Meshes[0]->Translate({ -1.75f, 4.5f, 0.f });
		m_Meshes[0]->CalculateNormals();
		m_Meshes[0]->UpdateAABB();
		m_Meshes[0]->UpdateTransforms();

		m_Meshes[1] = AddTriangleMesh(TriangleCullMode::FrontFaceCulling, matLambert_White);
		m_Meshes[1]->AppendTriangle(baseTriangle, true);
		m_Meshes[1]->Translate({ 0.f, 4.5f, 0.f });
		m_Meshes[1]->CalculateNormals();
		m_Meshes[1]->UpdateAABB();
		m_Meshes[1]->UpdateTransforms();

		m_Meshes[2] = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
		m_Meshes[2]->AppendTriangle(baseTriangle, true);
		m_Meshes[2]->Translate({ 1.75f, 4.5f, 0.f });
		m_Meshes[2]->CalculateNormals();
		m_Meshes[2]->UpdateAABB();
		m_Meshes[2]->UpdateTransforms();


		// Lights
		AddPointLight({ 0.f, 5.f, 5.f }, 50.f, { 1.f, .61f, .45f }); // BACKLIGHT
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, { 1.f, .8f, .45f }); // FRONT LIGHT LEFT
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, { 0.34f, .47f, .68f });

	}
	void Scene_W4_ReferenceScene::Update(dae::Timer* pTimer)
	{
		Scene::Update(pTimer);  // run base class function

		// Rotate the trianglemesh frame by frame
		// Ptimer gettotal time will increase the longer the scene runs (accumulated time)


		const float yawAngle = (cos(pTimer->GetTotal()) + 1.f) / 2.f * PI_2;
		for (TriangleMesh* mesh : m_Meshes)
		{
			mesh->RotateY(yawAngle);
			mesh->UpdateTransforms();
		}

	}
	void Scene_W4_BunnyScene::Initialize()
	{
		sceneName = "Bunny Scene";
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.SetFov(45.0f);

		// Materials
		const auto matCt_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.96f, 0.915f }, 1.f, 1.f));
		const auto matCt_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.96f, 0.915f }, 1.f, 0.6f));
		const auto matCt_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.96f, 0.915f }, 1.f, 0.1f));

		const auto matCt_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.f, 1.f));
		const auto matCt_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.f, 0.6f));
		const auto matCt_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.f, 0.1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ 0.49f, 0.57f, 0.57f }, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

		// Planes
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f }, matLambert_GrayBlue);	// BACK
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matLambert_GrayBlue);	// BOTTOM
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f }, matLambert_GrayBlue);  // TOP
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, matLambert_GrayBlue);	// RIGHT
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, matLambert_GrayBlue);	// LEFT

		// Bunny
		pMesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		//Utils::ParseOBJ("Resources/truck2.obj", pMesh->positions, pMesh->normals, pMesh->indices);
		Utils::ParseOBJ("Resources/lowpoly_bunny2.obj", pMesh->positions, pMesh->normals, pMesh->indices);

		//pMesh->CalculateNormals();
		pMesh->Scale({ 2.f, 2.f, 2.f });
		//pMesh->Scale({ 0.05f, 0.05f, 0.05f });
		//pMesh->Translate({ 0.f, 2.f, 0.f });

		pMesh->UpdateAABB();
		pMesh->UpdateTransforms();

		// Lights
		AddPointLight({ 0.f, 5.f, 5.f }, 50.f, { 1.f, .61f, .45f }); // BACKLIGHT
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, { 1.f, .8f, .45f }); // FRONT LIGHT LEFT
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, { 0.34f, .47f, .68f });
	}
	void Scene_W4_BunnyScene::Update(dae::Timer* pTimer)
	{
		Scene::Update(pTimer);
		
		const float yawAngle = (cos(pTimer->GetTotal()) + 1.f) / 2.f * PI_2;

		pMesh->RotateY(yawAngle);
		pMesh->UpdateTransforms();
	}

	void Scene_W4_BunnySceneReflections::Initialize()
	{
		sceneName = "Bunny Scene with mirror";
		m_ReflectionsEnabled = true;
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.SetFov(45.0f);

		// Materials
		//const auto matCt_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.96f, 0.915f }, 1.f, 1.f));
		//const auto matCt_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.96f, 0.915f }, 1.f, 0.6f));
		const auto matCt_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.96f, 0.915f }, 1.f, 0.1f));

		//const auto matCt_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.f, 1.f));
		//const auto matCt_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.f, 0.6f));
		//const auto matCt_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.f, 0.1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ 0.49f, 0.57f, 0.57f }, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

		// Planes
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f }, matLambert_GrayBlue);	// BACK
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matLambert_GrayBlue);	// BOTTOM
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f }, matLambert_GrayBlue);  // TOP
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, matCt_GraySmoothMetal);	// RIGHT
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, matLambert_GrayBlue);	// LEFT

		// Bunny
		pMesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		//Utils::ParseOBJ("Resources/truck2.obj", pMesh->positions, pMesh->normals, pMesh->indices);
		Utils::ParseOBJ("Resources/lowpoly_bunny2.obj", pMesh->positions, pMesh->normals, pMesh->indices);

		//pMesh->CalculateNormals();
		pMesh->Scale({ 2.f, 2.f, 2.f });
		//pMesh->Scale({ 0.05f, 0.05f, 0.05f });
		//pMesh->Translate({ 0.f, 2.f, 0.f });

		pMesh->UpdateAABB();
		pMesh->UpdateTransforms();

		// Lights
		AddPointLight({ 0.f, 5.f, 5.f }, 50.f, { 1.f, .61f, .45f }); // BACKLIGHT
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, { 1.f, .8f, .45f }); // FRONT LIGHT LEFT
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, { 0.34f, .47f, .68f });
	}
	void Scene_W4_BunnySceneReflections::Update(dae::Timer* pTimer)
	{
		Scene::Update(pTimer);

		const float yawAngle = (cos(pTimer->GetTotal()) + 1.f) / 2.f * PI_2;

		pMesh->RotateY(yawAngle);
		pMesh->UpdateTransforms();
	}
}
