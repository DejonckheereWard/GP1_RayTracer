//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;


Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
	RunTests();
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();


	float aspectRatio{ float(m_Width) / float(m_Height) };

	const float fovRatio{ tan(camera.fovAngle * TO_RADIANS / 2.0f) };
	const Matrix cameraToWorld = camera.CalculateCameraToWorld();

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			float cx{ ((2.0f * (px + 0.5f) / float(m_Width)) - 1) * aspectRatio * fovRatio };
			float cy{ (1.0f - ((2.0f * (py + 0.5f)) / float(m_Height))) * fovRatio };

			Vector3 rayDirection{ cameraToWorld.TransformVector(Vector3{cx, cy, 1}).Normalized() };
			Ray viewRay{ camera.origin,  rayDirection };

			ColorRGB finalColor{};

			HitRecord closestHit{};
			pScene->GetClosestHit(viewRay, closestHit);  // Checks EVERY object in the scene and returns the closest one hit.

			if (closestHit.didHit)
			{
				finalColor = materials[closestHit.materialIndex]->Shade();

				// Check if pixel is shadowed
				float TotalObservedArea{ 0.0f };
				for (const Light& light : lights)
				{
					// Calculate hit towards light ray
					// Use small offset for the ray origin (use normal direction)
					Vector3 lightDirection{ LightUtils::GetDirectionToLight(light, closestHit.origin) };
					const float lightDistance{ lightDirection.Normalize() };
					Ray lightRay{ closestHit.origin + closestHit.normal * 0.0001f, lightDirection, 0.0f, lightDistance };

					if (m_ShadowsEnabled && pScene->DoesHit(lightRay))
					{
						// Check if point can see the light
						continue;
					}

					// Calculate observed area (Lambert's cosine law)
					float observedArea{ Vector3::Dot(closestHit.normal, lightDirection) };
					if (observedArea < 0) continue;
					TotalObservedArea += observedArea;

					switch (m_CurrentLightingMode)
					{
					case dae::Renderer::LightingMode::ObservedArea:
					{
						finalColor = ColorRGB(TotalObservedArea, TotalObservedArea, TotalObservedArea);
						break;
					}
					case dae::Renderer::LightingMode::Radiance:
						break;
					case dae::Renderer::LightingMode::BRDF:
						break;
					case dae::Renderer::LightingMode::Combined:
						break;
					}

				}
			}


			//Update Color in Buffer
			finalColor.MaxToOne();
			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void dae::Renderer::CycleLightingMode()
{
	switch (m_CurrentLightingMode)
	{
	case dae::Renderer::LightingMode::ObservedArea:
		m_CurrentLightingMode = LightingMode::Radiance;
		std::cout << "LightingMode: Radiance\n";
		break;
	case dae::Renderer::LightingMode::Radiance:
		m_CurrentLightingMode = LightingMode::BRDF;
		std::cout << "LightingMode: BRDF\n";
		break;
	case dae::Renderer::LightingMode::BRDF:
		m_CurrentLightingMode = LightingMode::Combined;
		std::cout << "LightingMode: Combined\n";
		break;
	case dae::Renderer::LightingMode::Combined:
		m_CurrentLightingMode = LightingMode::ObservedArea;
		std::cout << "LightingMode: ObservedArea\n";
		break;
	}
}

int Renderer::RunTests()
{
	// Test dot & cross product for vector3 & vector4
	assert(int(roundf(Vector3::Dot(Vector3::UnitX, Vector3::UnitX))) == 1);  // Should be 1 -> same direction
	assert(int(roundf(Vector3::Dot(Vector3::UnitX, -Vector3::UnitX))) == -1);  // Should be -1 -> opposite direction
	assert(int(roundf(Vector3::Dot(Vector3::UnitX, Vector3::UnitY))) == 0);  // Should be 0 -> perpendicular direction


	return 0;
}