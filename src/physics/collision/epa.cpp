#include "epa.h"
#include <algorithm>


//helpers
std::pair<std::vector<glm::vec4>, size_t> GetFaceNormals(
    const std::vector<SupportPoint>& polytope,
    const std::vector<size_t>& faces
) {
    std::vector<glm::vec4> normals;
    size_t minTriangle = 0;
    float minDistance = FLT_MAX;

    for (size_t i=0;i<faces.size(); i+=3) {
        glm::vec3 a = polytope[faces[i]].point;
        glm::vec3 b = polytope[faces[i+1]].point;
        glm::vec3 c = polytope[faces[i+2]].point;

        glm::vec3 normal = glm::normalize(glm::cross(b-a,c-a));
        float distance = dot(normal, a);

        if (distance < 0) {
            normal *= -1;
            distance *= -1;
        }
        normals.emplace_back(glm::vec4(distance, normal));
        if (distance < minDistance) {
            minTriangle = i/3;
            minDistance = distance;
        }
    }
    return {normals, minTriangle};
}

void AddIfUniqueEdge(
	std::vector<std::pair<size_t, size_t>>& edges,
	const std::vector<size_t>& faces,
	size_t a,
	size_t b)
{
	auto reverse = std::find(                       //      0--<--3
		edges.begin(),                              //     / \ B /   A: 2-0
		edges.end(),                                //    / A \ /    B: 0-2
		std::make_pair(faces[b], faces[a]) //   1-->--2
	);
 
	if (reverse != edges.end()) {
		edges.erase(reverse);
	}
 
	else {
		edges.emplace_back(faces[a], faces[b]);
	}
}

bool SameDirection(glm::vec4 a, glm::vec3 b) {
    return dot(glm::vec3(a.x,a.y,a.z), b) > 0;
}


CollisionResult EPA(const Collider* a, const Collider* b, Simplex s) {
    std::vector<SupportPoint> polytope(s.points.begin(), s.points.end());
    
    std::vector<size_t> faces = {
        0,1,2,
        0,3,1,
        0,2,3,
        1,3,2,
    };
    auto [normals, minFace] = GetFaceNormals(polytope, faces);
    glm::vec3 minNormal;
    float minDistance = FLT_MAX;

    SupportPoint support;

    while(minDistance == FLT_MAX) {
        glm::vec4 temp = normals[minFace];
        minNormal = glm::vec3(temp.x, temp.y, temp.z);
        minDistance = temp.w;
        support = Support(a, b, minNormal);
        float sDistance = dot(minNormal, support.point);

        if (abs(sDistance - minDistance) < 0.001f){
            minDistance = FLT_MAX;

            std::vector<std::pair<size_t, size_t>> uniqueEdges;
			for (size_t i = 0; i < normals.size(); i++) {
				if (SameDirection(normals[i], support.point)) {
					size_t f = i * 3;

					AddIfUniqueEdge(uniqueEdges, faces, f,     f + 1);
					AddIfUniqueEdge(uniqueEdges, faces, f + 1, f + 2);
					AddIfUniqueEdge(uniqueEdges, faces, f + 2, f    );

					faces[f + 2] = faces.back(); faces.pop_back();
					faces[f + 1] = faces.back(); faces.pop_back();
					faces[f    ] = faces.back(); faces.pop_back();

					normals[i] = normals.back(); // pop-erase
					normals.pop_back();

					i--;
				}
			}

            std::vector<size_t> newFaces;
			for (auto [edgeIndex1, edgeIndex2] : uniqueEdges) {
				newFaces.push_back(edgeIndex1);
				newFaces.push_back(edgeIndex2);
				newFaces.push_back(polytope.size());
			}
			 
			polytope.push_back(support);

			auto [newNormals, newMinFace] = GetFaceNormals(polytope, newFaces);
            float oldMinDistance = FLT_MAX;
			for (size_t i = 0; i < normals.size(); i++) {
				if (normals[i].w < oldMinDistance) {
					oldMinDistance = normals[i].w;
					minFace = i;
				}
			}
 
			if (newNormals[newMinFace].w < oldMinDistance) {
				minFace = newMinFace + normals.size();
			}
 
			faces  .insert(faces  .end(), newFaces  .begin(), newFaces  .end());
			normals.insert(normals.end(), newNormals.begin(), newNormals.end());
        }
    }
    CollisionResult info;
 
	info.normal = minNormal;
	info.penetration = minDistance + 0.001f;
    info.contactA = support.suppA;
    info.contactB = support.suppB;

    return info;
 
}