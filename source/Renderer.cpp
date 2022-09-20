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


Renderer::Renderer(SDL_Window * pWindow) :
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


	float aspectRatio{ float(m_Width) / float(m_Height)};


	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			float gradient = px / static_cast<float>(m_Width);
			gradient += py / static_cast<float>(m_Width);
			gradient /= 2.0f;

			float cx{ ((2.0f * (px + 0.5f) / float(m_Width)) - 1) * aspectRatio };
			float cy{ 1.0f - ((2.0f * (py + 0.5f)) / float(m_Height)) };

			Vector3 rayDirection{};
			rayDirection = cx * camera.right + cy * camera.up + 1.0f * camera.forward;
			rayDirection.Normalize();
			
			Ray hitRay{ camera.origin,  rayDirection };


			ColorRGB finalColor{};


			HitRecord closestHit{};
			pScene->GetClosestHit(hitRay, closestHit);

			//Sphere testSphere{ Vector3{ 0.0f, 0.0f, 100.0f }, 50.0f, 0 };

			//GeometryUtils::HitTest_Sphere(testSphere, hitRay, closestHit);
			if (closestHit.didHit)
			{

				//const float scaledT{ (closestHit.t - 50.0f) / 40.0f};
				//finalColor = {scaledT, scaledT, scaledT};
				finalColor = materials[closestHit.materialIndex]->Shade();
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

int Renderer::RunTests()
{
	// Test dot & cross product for vector3 & vector4
	assert(int(roundf(Vector3::Dot(Vector3::UnitX, Vector3::UnitX))) == 1);  // Should be 1 -> same direction
	assert(int(roundf(Vector3::Dot(Vector3::UnitX, -Vector3::UnitX))) == -1);  // Should be -1 -> opposite direction
	assert(int(roundf(Vector3::Dot(Vector3::UnitX, Vector3::UnitY))) == 0);  // Should be 0 -> perpendicular direction


	return 0;
}