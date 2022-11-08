#pragma once

#include <cstdint>
#include <vector>
#include "Math.h"

struct SDL_Window;
struct SDL_Surface;
namespace dae
{
	class Scene;
	struct Camera;
	struct Light;
	class Material;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene);
		
		void RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, 
			const Camera& camera, const std::vector<Light>& lights, const std::vector<Material*>& materials) const;

		bool SaveBufferToImage() const;

		void CycleLightingMode();
		void ToggleShadows() { m_ShadowsEnabled = !m_ShadowsEnabled; }
		void ToggleReflections() { m_ReflectionsEnabled = !m_ReflectionsEnabled; }
		void SetReflections(bool value) { m_ReflectionsEnabled = value; }
		const std::vector<Vector3>& GetRayDirections() const { return m_RayDirections; }
		void RecalculateRayDirections(Camera& camera);

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		int m_Width{};
		int m_Height{};
		float m_AspectRatio{};
		int m_Bounces{ 3 };
		std::vector<Vector3> m_RayDirections;

		enum class LightingMode
		{
			ObservedArea, // Lambert cosine law
			Radiance, // Incident Radiance
			BRDF, // Scattering of the light
			Combined // ObservedArea & Radiance & BRDF
		};

		LightingMode m_CurrentLightingMode{ LightingMode::Combined };
		bool m_ShadowsEnabled{ true };
		bool m_ReflectionsEnabled{ false };


		static bool RunTests();
	};
}
