#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"
#include <iostream>

#define MOLLER_TRUMBORE

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
			float dp{ Vector3::Dot(tc, ray.direction) };  // Vector TP  (P is perpendicular to the raycast, and goes to C)
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

			// Check if T exceeds the boundaries set in the ray struct (tMin & tMax)
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

#ifdef MOLLER_TRUMBORE
			// Möller–Trumbore intersection algorithm
			const Vector3 edge1{ triangle.v1 - triangle.v0 };
			const Vector3 edge2{ triangle.v2 - triangle.v0 };

			const Vector3 h{ Vector3::Cross(ray.direction, edge2) };
			const float a{ Vector3::Dot(edge1, h) };

			if (AreEqual(a, 0.0f))
				return false;

			if (a < 0.0f)
			{
				// Backface hit
				if (!ignoreHitRecord && triangle.cullMode == TriangleCullMode::BackFaceCulling)
					// Remove the face if it's "culled" away
					return false;
				if (ignoreHitRecord && triangle.cullMode == TriangleCullMode::FrontFaceCulling)
					// Shadow rays (ignorehitrecord true) have inverted culling
					return false;
			}
			else if (a > 0.0f)
			{
				// Frontface hit
				if (!ignoreHitRecord && triangle.cullMode == TriangleCullMode::FrontFaceCulling)
					// Remove the face if it's "culled" away
					return false;
				if (ignoreHitRecord && triangle.cullMode == TriangleCullMode::BackFaceCulling)
					// Shadow rays (ignorehitrecord true) have inverted culling
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
				hitRecord.normal = triangle.normal;
				hitRecord.t = t;
				return true;
			}
			return false;
#else
			// Clockwise all edges of the triangle
			const Vector3 edgeA{ triangle.v1 - triangle.v0 };
			const Vector3 edgeB{ triangle.v2 - triangle.v1 };
			const Vector3 edgeC{ triangle.v0 - triangle.v2 };

			// Cross the 2 edges to get the normal of the triangle
			const Vector3 normal{ triangle.normal };

			// Check if the ray is parallel to the triangle
			const float NdotV{ Vector3::Dot(ray.direction, normal) };
			if (NdotV == 0)
			{
				return false;  // If the ray faces away from the plane of the triangle, it won't ever hit
			}
			else if (NdotV > 0)
			{
				// BACK FACE towards us
				if (!ignoreHitRecord && triangle.cullMode == TriangleCullMode::BackFaceCulling)
				{
					// Don't render the back face if we cull the back face (culling == strip/remove)
					return false;
				}
				else if (ignoreHitRecord && triangle.cullMode == TriangleCullMode::FrontFaceCulling)
				{
					// Shadowrays are inverted so we invert the culling for shadowrays (ignorehitrecord true)
					return false;
				}
			}
			else
			{
				// FRONT FACE towards us
				if (!ignoreHitRecord && triangle.cullMode == TriangleCullMode::FrontFaceCulling)
				{
					// Don't render the front face if we cull the front face
					return false;
				}

				if (ignoreHitRecord && triangle.cullMode == TriangleCullMode::BackFaceCulling)
				{
					// If it's a shadow ray (ignorehitrecord true), we invert the culling
					return false;
				}
			}


			// Average of the 3 points to get the center of the triangle
			const Vector3 center{ (triangle.v0 + triangle.v1 + triangle.v2) / 3.0f };

			// Calculate distance hitPoint
			const Vector3 l{ center - ray.origin };  // Point to the center of the 'plane'
			const float t{ Vector3::Dot(l, normal) / NdotV };

			// Check if T exceeds the boundaries set in the ray struct (tMin & tMax)
			if (t < ray.min || t > ray.max)
				return false;  // T is out of bounds

			// We can calculate where point P is, by multiplying the direction, with the distance (t) found earlier.
			// Add that to the ray's origin to find P
			const Vector3 p{ ray.origin + (t * ray.direction) };  // Point on the plane


			// Now we check wether the found point is inside or outside the triangle bounds
			//const Vector3 pointToSide{ p - triangle.v0 };

			if (Vector3::Dot(normal, Vector3::Cross(edgeA, p - triangle.v0)) < 0)
				return false;  // Point is outside the triangle

			if (Vector3::Dot(normal, Vector3::Cross(edgeB, p - triangle.v1)) < 0)
				return false;  // Point is outside the triangle

			if (Vector3::Dot(normal, Vector3::Cross(edgeC, p - triangle.v2)) < 0)
				return false;  // Point is outside the triangle

			if (ignoreHitRecord)
				return true;

			hitRecord.didHit = true;
			hitRecord.materialIndex = triangle.materialIndex;
			hitRecord.origin = p;
			hitRecord.normal = normal;
			hitRecord.t = t;

			return true;
#endif
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool SlabTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			// Perform slabtest on the mesh (acceleration structures)

			//float tmin{FLT_MAX};
			//float tmax{-FLT_MAX};

			//const Vector3 t1{ (mesh.transformedMinAABB - ray.origin) };
			//const Vector3 t2{ (mesh.transformedMaxAABB - ray.origin) };

			//const float tx1{ t1.x / ray.direction.x };
			//const float tx2{ t2.x / ray.direction.x };
			//tmin = std::min(tmin, std::min(tx1, tx2));
			//tmax = std::max(tmax, std::max(tx1, tx2));

			//const float ty1{ t1.y / ray.direction.y };
			//const float ty2{ t2.y / ray.direction.y };
			//tmin = std::min(tmin, std::min(ty1, ty2));
			//tmax = std::max(tmax, std::max(ty1, ty2));

			//const float tz1{ t1.z / ray.direction.z };
			//const float tz2{ t1.z / ray.direction.z };
			//
			//tmin = std::min(tmin, std::min(tz1, tz2));
			//tmax = std::max(tmax, std::max(tz1, tz2));

			//return tmax > 0 && tmax >= tmin;

			float tx1 = (mesh.transformedMinAABB.x - ray.origin.x) / ray.direction.x;
			float tx2 = (mesh.transformedMaxAABB.x - ray.origin.x) / ray.direction.x;

			float tmin = std::min(tx1, tx2);
			float tmax = std::max(tx1, tx2);

			float ty1 = (mesh.transformedMinAABB.y - ray.origin.y) / ray.direction.y;
			float ty2 = (mesh.transformedMaxAABB.y - ray.origin.y) / ray.direction.y;

			tmin = std::max(tmin, std::min(ty1, ty2));
			tmax = std::min(tmax, std::max(ty1, ty2));

			float tz1 = (mesh.transformedMinAABB.z - ray.origin.z) / ray.direction.z;
			float tz2 = (mesh.transformedMaxAABB.z - ray.origin.z) / ray.direction.z;

			tmin = std::max(tmin, std::min(tz1, tz2));
			tmax = std::min(tmax, std::max(tz1, tz2));

			return tmax > 0 && tmax >= tmin;

		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false, bool closestHit = false)
		{
			// Opitimization using slabtest
			// Checks if ray hits the slab/bounding box (AABB), stops the calculation if ray doesn't hit this box
			if (!SlabTest_TriangleMesh(mesh, ray))
				return false;

			// Loop through all triangles in the mesh, and check if they hit the ray.
			Triangle triangle{};
			const size_t trianglePositionsSize{ mesh.positions.size() };
			const size_t meshIndicesSize{ mesh.indices.size() };

			for (size_t i{}; i < meshIndicesSize; i += 3)
			{
				const Vector3 v0{ mesh.transformedPositions[mesh.indices[i]] };
				const Vector3 v1{ mesh.transformedPositions[mesh.indices[i + 1]] };
				const Vector3 v2{ mesh.transformedPositions[mesh.indices[i + 2]] };
				Triangle triangle(v0, v1, v2, mesh.transformedNormals[i / 3]);
				triangle.cullMode = mesh.cullMode;
				triangle.materialIndex = mesh.materialIndex;

				HitRecord tempHitrecord{};
				if (HitTest_Triangle(triangle, ray, tempHitrecord, ignoreHitRecord))
				{
					if (!closestHit)
					{
						hitRecord = tempHitrecord;
						return true;
					}
					else
					{
						if (tempHitrecord.t > 0.0f && tempHitrecord.t < hitRecord.t)
						{
							hitRecord = tempHitrecord;
						}
					}

				}
			}
			return hitRecord.didHit;
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
				const float sphereRadiusSquared((light.origin - target).SqrMagnitude());  // Radius is the distance from the light to the target
				const float irradiance{ radiantPower / sphereRadiusSquared };

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