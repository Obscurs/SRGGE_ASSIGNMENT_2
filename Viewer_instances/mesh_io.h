// Author: Marc Comino 2020

#ifndef MESH_IO_H_
#define MESH_IO_H_

#include <triangle_mesh.h>
#include <memory>
#include <string>

namespace data_representation {
//Structures for Octree
class Node {
public:
  int depth_;
  int vertex_index_;
  //node* children_[8];
  Node* father_;

  Node(const std::vector<float> &vertices, const std::vector<int> &currentIndices, std::vector<Node*> &nodes, int depth, Node* father,  double offset_x, double offset_y, double offset_z, double size_x, double size_y, double size_z);
  
  static void deleteOctree(std::vector<Node*> &nodes);
  
};
/**
 * @brief ReadFromPly Read the mesh stored in PLY format at the path filename
 * and stores the corresponding TriangleMesh representation
 * @param filename The path to the PLY mesh.
 * @param mesh The resulting representation with computed per-vertex normals.
 * @return Whether it was able to read the file.
 */
bool ReadFromPly(const std::string &filename, TriangleMesh *mesh);

/**
 * @brief WriteToPly Stores the mesh representation in PLY format at the path
 * filename.
 * @param filename The path where the mesh will be stored.
 * @param mesh The mesh to be stored.
 * @return Whether it was able to store the file.
 */
bool WriteToPly(const std::string &filename, const TriangleMesh &mesh);

void GenLodMeshQuadratics(float cellSize, const TriangleMesh *mesh, TriangleMesh *newMesh);
void GenLodMeshAverage(float cellSize, const TriangleMesh *mesh, TriangleMesh *newMesh);
void GenLodMeshAverageOctree(int maxDepth, const TriangleMesh *mesh, TriangleMesh *newMesh, const std::vector<Node*> &octree);
Node* GenOctree(const TriangleMesh *mesh, std::vector<Node*> &octree);

}  // namespace data_representation





#endif  // MESH_IO_H_
