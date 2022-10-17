#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"
#include <iostream>

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
#pragma region Geometric
			//Vector from ray origin to center of sphere
			Vector3 tc{ sphere.origin - ray.origin };   // Vector TC  (T is start, C is center of sphere)

			//Vector3 a{ Vector3::Dot(ray.direction, ray.direction) };
			float dp{ Vector3::Dot(tc, ray.direction)};  // Vector TP  (P is perpendicular to the raycast, and goes to C)
			float odSqr{ tc.SqrMagnitude() - Square(dp) };  // Power of length between P & C
			if (odSqr > Square(sphere.radius))
			{
				// Optimization, if odSqr is larger than radius square, it's a miss.
				return false;
			}
			float tca{ sqrtf(Square(sphere.radius) - odSqr) };  // Distance I1 P
			float ti1{ dp - tca };  // Distance from origin to Intersection Point 1

			
			if (ti1 >= ray.min && ti1 <= ray.max)
			{
				if (ignoreHitRecord) return true;
				
				const Vector3 pointI1{ ray.origin + ray.direction * ti1 };  // Point I1
				hitRecord.didHit = true;
				hitRecord.materialIndex = sphere.materialIndex;
				hitRecord.origin = pointI1;
				hitRecord.normal = (pointI1 - sphere.origin).Normalized();
				hitRecord.t = ti1;
				return true;
			}
			return false;
#pragma endregion
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W1

			// Check if ray hits the plane, and where the hit is.

			// Calculate distance hitPoint
			const float t{ Vector3::Dot((plane.origin - ray.origin), plane.normal) / Vector3::Dot(ray.direction, plane.normal) };

			// Cehck if T exceeds the boundaries set in the ray struct (tMin & tMax)
			if ((t >= ray.min) && (t <= ray.max))
			{
				// We can calculate where point P is, by multiplying the direction, with the distance (t) found earlier.
				// Add that to the ray's origin to find P
				if (ignoreHitRecord) return true;
				hitRecord.didHit = true;
				hitRecord.materialIndex = plane.materialIndex;
				hitRecord.normal = plane.normal;
				hitRecord.origin = ray.origin + (t * ray.direction);
				hitRecord.t = t;
				return true;

			}
			return false;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			//assert(false && "No Implemented Yet!");

			// Möller–Trumbore intersection algorithm

			const Vector3 edge1{ triangle.v1 - triangle.v0 };
			const Vector3 edge2{ triangle.v2 - triangle.v0 };

			const Vector3 h{ Vector3::Cross(ray.direction, triangle.v2 ) };
			const float a{ Vector3::Dot(edge1, ray.direction) };
			if (AreEqual(a, 0.0f))
			{
				return false;
			}
			const float f{ 1.0f / a };
			const Vector3 s{ ray.origin - triangle.v0 };
			const float u{ f * Vector3::Dot(s, h) };
			if (u < 0.0f || u > 1.0f)
				return false;
			const Vector3 q{ Vector3::Cross(s, edge1) };
			const float v{ f * Vector3::Dot(ray.direction, q) };
			if (v < 0.0f || u + v > 1.0f)
				return false;
			const float t{ f * Vector3::Dot(edge2, q) };
			if (t > ray.min && t < ray.max)
			{
				if (ignoreHitRecord) return true;
				hitRecord.didHit = true;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.origin = ray.origin + (t * ray.direction);
				hitRecord.normal = Vector3::Cross(edge1, edge2).Normalized();
				hitRecord.t = t;
				return true;
			}
			return false;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			//assert(false && "No Implemented Yet!");

			// Loop through all triangles in the mesh, and check if they hit the ray.
			Triangle triangle{};
			const size_t trianglePositionsSize{ mesh.positions.size() };

			for (size_t i{ 0 }; i < trianglePositionsSize; i += 3)
			{
				triangle.v0 = mesh.positions[i];
				triangle.v1 = mesh.positions[i + 1];
				triangle.v2 = mesh.positions[i + 2];
				triangle.materialIndex = mesh.materialIndex;
				if (HitTest_Triangle(triangle, ray, hitRecord, ignoreHitRecord))
				{
					return true;
				}
			}
			return false;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			// Returns unnormalized vector from the origin to the lights origin
			// Implementation depends on the type of light
			switch (light.type)
			{
			case dae::LightType::Point:
				return light.origin - origin;
				break;
			case dae::LightType::Directional:
				// Directional lights have no origin, so we just return the direction
				// Magnitude of a directional light is max value we can give: FLT_MAX
				return -light.direction * FLT_MAX;
				break;
			default:
				return -light.direction * FLT_MAX;
				break;
			}
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			// Radiant Intensity (which we already know 'light.intensity') combined with the Irradiance.
			switch (light.type)
			{
			case dae::LightType::Point:
			{				
				// Calculate the Radiant Power/Flux, Since it's a point light, we multiply the intensity with 4PI sterradians
				// 4pi steradians is the area of a whole 3d unit sphere  (since point lights emit in every direction)
				// We can cancel out the surface area to get the irradiance
				const float radiantPower{ light.intensity };  // also called Radiant Flux
				const float sphereRadiusSquared( (light.origin - target).SqrMagnitude());  // Radius is the distance from the light to the target
				const float irradiance{ radiantPower / sphereRadiusSquared};

				return light.color * irradiance;  // Irradiancecolor
				break;
			}
			case dae::LightType::Directional:
				// Directional lights are a bit simpler, they don't have any fallof or attenuation nor area, so we just return the intensity with the lightcolor
				return light.color * light.intensity;
				break;
			}			
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof())
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if (isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}