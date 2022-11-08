#pragma once
#include <string>
#include <vector>

#include "Math.h"
#include "DataTypes.h"
#include "Camera.h"

namespace dae
{
	//Forward Declarations
	class Timer;
	class Material;
	struct Plane;
	struct Sphere;
	struct Light;

	//Scene Base Class
	
	class Scene
	{
	public:
		Scene();
		virtual ~Scene();

		Scene(const Scene&) = delete;
		Scene(Scene&&) noexcept = delete;
		Scene& operator=(const Scene&) = delete;
		Scene& operator=(Scene&&) noexcept = delete;

		virtual void Initialize() = 0;
		virtual void Update(dae::Timer* pTimer)
		{
			m_Camera.Update(pTimer);
		}

		Camera& GetCamera() { return m_Camera; }
		void GetClosestHit(const Ray& ray, HitRecord& closestHit) const;
		bool DoesHit(const Ray& ray) const;
		bool GetReflectionsEnabled() const { return m_ReflectionsEnabled; }

		const std::vector<Plane>& GetPlaneGeometries() const { return m_PlaneGeometries; }
		const std::vector<Sphere>& GetSphereGeometries() const { return m_SphereGeometries; }
		const std::vector<Light>& GetLights() const { return m_Lights; }
		const std::vector<Material*> GetMaterials() const { return m_Materials; }

	protected:
		std::string	sceneName;

		std::vector<Plane> m_PlaneGeometries{};
		std::vector<Sphere> m_SphereGeometries{};
		std::vector<TriangleMesh> m_TriangleMeshGeometries{};
		std::vector<Light> m_Lights{};
		std::vector<Material*> m_Materials{};
		
		bool m_ReflectionsEnabled{};
		
		Camera m_Camera{};

		Sphere* AddSphere(const Vector3& origin, float radius, unsigned char materialIndex = 0);
		Plane* AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex = 0);
		TriangleMesh* AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex = 0);

		Light* AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color);
		Light* AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color);
		unsigned char AddMaterial(Material* pMaterial);
	};

	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 1 Test Scene
	class Scene_W1 final : public Scene
	{
	public:
		Scene_W1() = default;
		~Scene_W1() override = default;

		Scene_W1(const Scene_W1&) = delete;
		Scene_W1(Scene_W1&&) noexcept = delete;
		Scene_W1& operator=(const Scene_W1&) = delete;
		Scene_W1& operator=(Scene_W1&&) noexcept = delete;

		void Initialize() override;
	};

	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 2 Test Scene
	class Scene_W2 final : public Scene
	{
	public:
		Scene_W2() = default;
		~Scene_W2() override = default;

		Scene_W2(const Scene_W2&) = delete;
		Scene_W2(Scene_W2&&) noexcept = delete;
		Scene_W2& operator=(const Scene_W2&) = delete;
		Scene_W2& operator=(Scene_W2&&) noexcept = delete;

		virtual void Update(dae::Timer* pTimer) override;

		void Initialize() override;
	private:
		unsigned char matId_Changing_Color{};
		int currentColorOffset{ 0 };
	};

	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 3 Test Scene
	class Scene_W3 final : public Scene
	{
	public:
		Scene_W3() = default;
		~Scene_W3() override = default;

		Scene_W3(const Scene_W3&) = delete;
		Scene_W3(Scene_W3&&) noexcept = delete;
		Scene_W3& operator=(const Scene_W3&) = delete;
		Scene_W3& operator=(Scene_W3&&) noexcept = delete;

		void Initialize() override;
	};
	
	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 3 Test Scene
	class Scene_W3_Test final : public Scene
	{
	public:
		Scene_W3_Test() = default;
		~Scene_W3_Test() override = default;

		Scene_W3_Test(const Scene_W3_Test&) = delete;
		Scene_W3_Test(Scene_W3_Test&&) noexcept = delete;
		Scene_W3_Test& operator=(const Scene_W3_Test&) = delete;
		Scene_W3_Test& operator=(Scene_W3_Test&&) noexcept = delete;

		void Initialize() override;
	};

	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 4 Test Scene
	class Scene_W4_TestScene final : public Scene
	{
	public:
		Scene_W4_TestScene() = default;
		~Scene_W4_TestScene() override = default;

		Scene_W4_TestScene(const Scene_W4_TestScene&) = delete;
		Scene_W4_TestScene(Scene_W4_TestScene&&) noexcept = delete;
		Scene_W4_TestScene& operator=(const Scene_W4_TestScene&) = delete;
		Scene_W4_TestScene& operator=(Scene_W4_TestScene&&) noexcept = delete;

		void Initialize() override;
		void Update(dae::Timer* pTimer) override;

	private:
		TriangleMesh* pMesh{ nullptr };
	};

	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 4 Reference Scene
	class Scene_W4_ReferenceScene final : public Scene
	{
	public:
		Scene_W4_ReferenceScene() = default;
		~Scene_W4_ReferenceScene() override = default;

		Scene_W4_ReferenceScene(const Scene_W4_ReferenceScene&) = delete;
		Scene_W4_ReferenceScene(Scene_W4_ReferenceScene&&) noexcept = delete;
		Scene_W4_ReferenceScene& operator=(const Scene_W4_ReferenceScene&) = delete;
		Scene_W4_ReferenceScene& operator=(Scene_W4_ReferenceScene&&) noexcept = delete;

		void Initialize() override;
		void Update(dae::Timer* pTimer) override;

	private:
		TriangleMesh* m_Meshes[3]{};

	};

	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 4 Reference Scene
	class Scene_W4_BunnyScene final : public Scene
	{
	public:
		Scene_W4_BunnyScene() = default;
		~Scene_W4_BunnyScene() override = default;

		Scene_W4_BunnyScene(const Scene_W4_BunnyScene&) = delete;
		Scene_W4_BunnyScene(Scene_W4_BunnyScene&&) noexcept = delete;
		Scene_W4_BunnyScene& operator=(const Scene_W4_BunnyScene&) = delete;
		Scene_W4_BunnyScene& operator=(Scene_W4_BunnyScene&&) noexcept = delete;

		void Initialize() override;
		void Update(dae::Timer* pTimer) override;

	private:
		TriangleMesh* pMesh{ nullptr };

	};
	
	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 4 Reference Scene
	class Scene_W4_BunnySceneReflections final : public Scene
	{
	public:
		Scene_W4_BunnySceneReflections() = default;
		~Scene_W4_BunnySceneReflections() override = default;

		Scene_W4_BunnySceneReflections(const Scene_W4_BunnySceneReflections&) = delete;
		Scene_W4_BunnySceneReflections(Scene_W4_BunnySceneReflections&&) noexcept = delete;
		Scene_W4_BunnySceneReflections& operator=(const Scene_W4_BunnySceneReflections&) = delete;
		Scene_W4_BunnySceneReflections& operator=(Scene_W4_BunnySceneReflections&&) noexcept = delete;

		void Initialize() override;
		void Update(dae::Timer* pTimer) override;

	private:
		TriangleMesh* pMesh{ nullptr };

	};

}
