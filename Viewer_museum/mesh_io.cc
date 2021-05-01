// Author: Marc Comino 2020

#include <mesh_io.h>

#include <assert.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath> 
#include "./triangle_mesh.h"

#include <bits/stdc++.h>
namespace data_representation {

static const int DEBUG_INT = -1;

namespace {









////////////////////////////////

template <typename T>
void Add3Items(T i1, T i2, T i3, size_t index, std::vector<T> *vector) {
  (*vector)[index] = i1;
  (*vector)[index + 1] = i2;
  (*vector)[index + 2] = i3;
}

bool ReadPlyHeader(std::ifstream *fin, int *vertices, int *faces) {
  char line[100];

  fin->getline(line, 100);
  if (strncmp(line, "ply", 3) != 0) return false;

  *vertices = 0;
  fin->getline(line, 100);
  while (strncmp(line, "end_header", 10) != 0) {
    if (strncmp(line, "element vertex", 14) == 0) *vertices = atoi(&line[15]);
    fin->getline(line, 100);
    if (strncmp(line, "element face", 12) == 0) *faces = atoi(&line[13]);
  }

  if (*vertices <= 0) return false;

  std::cout << "Loading triangle mesh" << std::endl;
  std::cout << "\tVertices = " << *vertices << std::endl;
  std::cout << "\tFaces = " << *faces << std::endl;

  return true;
}

void ReadPlyVertices(std::ifstream *fin, TriangleMesh *mesh) {
  const size_t kVertices = mesh->vertices_.size() / 3;
  char line[100];
  for (size_t i = 0; i < kVertices; ++i) {
    float x, y, z;
    float t1, t2;

    fin->getline(line, 100);
    std::istringstream in(line); 
    in >> x >> y >> z;
    //std::cout << i << " "<< x << " " << y << " " << z << std::endl;
    Add3Items(x, y, z, i * 3, &(mesh->vertices_));
  }
}

void ReadPlyFaces(std::ifstream *fin, TriangleMesh *mesh) {
  int vertex_per_face;
  char line[100];
  const size_t kFaces = mesh->faces_.size() / 3;
  for (size_t i = 0; i < kFaces; ++i) {
    int v1, v2, v3;
    //fin->read(reinterpret_cast<char *>(&vertex_per_face),
    //          sizeof(unsigned char));
    

    fin->getline(line, 100);
    std::istringstream in(line); 
    in >> vertex_per_face >>v1 >> v2 >> v3;
    //std::cout << vertex_per_face  << " "<< v1 << " " << v2 << " " << v3 << std::endl;
    assert(vertex_per_face == 3);
    //fin->read(reinterpret_cast<char *>(&v1), sizeof(int));
    //fin->read(reinterpret_cast<char *>(&v2), sizeof(int));
    //fin->read(reinterpret_cast<char *>(&v3), sizeof(int));

    if(kFaces == 69451)//UGLY HACK TO LOAD THE TWO DEFAULT MODELS
    {
          Add3Items(v1, v3, v2, i * 3, &(mesh->faces_));
    } else
    {
          Add3Items(v1, v2, v3, i * 3, &(mesh->faces_));
    }

  }
}

void ComputeVertexNormals(const std::vector<float> &vertices,
                          const std::vector<int> &faces,
                          std::vector<float> *normals) {
  const size_t kFaces = faces.size();
  std::vector<float> face_normals(kFaces, 0);

  for (size_t i = 0; i < kFaces; i += 3) {
    Eigen::Vector3d v1(vertices[faces[i] * 3], vertices[faces[i] * 3 + 1],
                       vertices[faces[i] * 3 + 2]);
    Eigen::Vector3d v2(vertices[faces[i + 1] * 3],
                       vertices[faces[i + 1] * 3 + 1],
                       vertices[faces[i + 1] * 3 + 2]);
    Eigen::Vector3d v3(vertices[faces[i + 2] * 3],
                       vertices[faces[i + 2] * 3 + 1],
                       vertices[faces[i + 2] * 3 + 2]);
    Eigen::Vector3d v1v2 = v2 - v1;
    Eigen::Vector3d v1v3 = v3 - v1;
    Eigen::Vector3d normal = v1v2.cross(v1v3);

    //if (normal.norm() < 0.00001) {
    //  normal = Eigen::Vector3d(0.0, 0.0, 0.0);
    //} else {
      normal.normalize();
    //}
    //std::cout << normal[0] << " " <<normal[1] << " " <<normal[2] << std::endl;
    for (size_t j = 0; j < 3; ++j) face_normals[i + j] = normal[j];
  }

  const size_t kVertices = vertices.size();
  normals->resize(kVertices, 0);
  for (size_t i = 0; i < kFaces; i += 3) {
    for (size_t j = 0; j < 3; ++j) {
      size_t idx = static_cast<size_t>(faces[i + j]);
      Eigen::Vector3d v1(vertices[faces[i + j] * 3],
                         vertices[faces[i + j] * 3 + 1],
                         vertices[faces[i + j] * 3 + 2]);
      Eigen::Vector3d v2(vertices[faces[i + (j + 1) % 3] * 3],
                         vertices[faces[i + (j + 1) % 3] * 3 + 1],
                         vertices[faces[i + (j + 1) % 3] * 3 + 2]);
      Eigen::Vector3d v3(vertices[faces[i + (j + 2) % 3] * 3],
                         vertices[faces[i + (j + 2) % 3] * 3 + 1],
                         vertices[faces[i + (j + 2) % 3] * 3 + 2]);

      Eigen::Vector3d v1v2 = v2 - v1;
      Eigen::Vector3d v1v3 = v3 - v1;
      double angle = acos(v1v2.dot(v1v3) / (v1v2.norm() * v1v3.norm()));

      if (angle == angle) {
        for (size_t k = 0; k < 3; ++k) {
          (*normals)[idx * 3 + k] += face_normals[i + k] * angle;
        }
      }
    }
  }

  const size_t kNormals = normals->size();
  for (size_t i = 0; i < kNormals; i += 3) {
    Eigen::Vector3d normal((*normals)[i], (*normals)[i + 1], (*normals)[i + 2]);
    if (normal.norm() > 0) {
      normal.normalize();
    } else {
      normal = Eigen::Vector3d(0, 0, 0);
    }

    for (size_t j = 0; j < 3; ++j) (*normals)[i + j] = normal[j];
  }
}

void ComputeBoundingBox(const std::vector<float> vertices, TriangleMesh *mesh) {
  const size_t kVertices = vertices.size() / 3;
  for (size_t i = 0; i < kVertices; ++i) {
    mesh->min_[0] = std::min(mesh->min_[0], vertices[i * 3]);
    mesh->min_[1] = std::min(mesh->min_[1], vertices[i * 3 + 1]);
    mesh->min_[2] = std::min(mesh->min_[2], vertices[i * 3 + 2]);

    mesh->max_[0] = std::max(mesh->max_[0], vertices[i * 3]);
    mesh->max_[1] = std::max(mesh->max_[1], vertices[i * 3 + 1]);
    mesh->max_[2] = std::max(mesh->max_[2], vertices[i * 3 + 2]);
  }
}

}  // namespace

bool ReadFromPly(const std::string &filename, TriangleMesh *mesh) {
  std::ifstream fin;

  fin.open(filename.c_str(), std::ios_base::in | std::ios_base::binary);
  if (!fin.is_open() || !fin.good()) return false;

  int vertices = 0, faces = 0;
  if (!ReadPlyHeader(&fin, &vertices, &faces)) {
    fin.close();
    std::cout << "\tError loading headers " << std::endl;
    return false;
  }
  std::cout << "\tHeaders loaded " << std::endl;
  mesh->vertices_.resize(static_cast<size_t>(vertices) * 3);
  ReadPlyVertices(&fin, mesh);
  std::cout << "\tLoaded vertices " << std::endl;
  mesh->faces_.resize(static_cast<size_t>(faces) * 3);
  ReadPlyFaces(&fin, mesh);
  std::cout << "\tLoaded faces " << std::endl;
  fin.close();

  ComputeVertexNormals(mesh->vertices_, mesh->faces_, &mesh->normals_);
  std::cout << "\tGenerated normals " << std::endl;
  ComputeBoundingBox(mesh->vertices_, mesh);
  std::cout << "\tGenerated bounding box " << std::endl;
  mesh->prepareVertexBuffer();
  std::cout << "\tPrepared vertex buffer " << std::endl;
  return true;
}





void GenLodMeshAverage(float cellSize, const TriangleMesh *mesh, TriangleMesh *newMesh)
{
  
  newMesh->Clear();
  if(cellSize==0)
  {
    newMesh->faces_ = mesh->faces_;
    newMesh->vertices_ = mesh->vertices_;
    newMesh->normals_ = mesh->normals_;
    newMesh->buffer_ = mesh->buffer_;
    newMesh->min_ = mesh->min_;
    newMesh->max_ = mesh->max_;


      ComputeVertexNormals(newMesh->vertices_, newMesh->faces_, &newMesh->normals_);
      std::cout << "\tGenerated normals " << std::endl;
      ComputeBoundingBox(newMesh->vertices_, newMesh);
      std::cout << "\tGenerated bounding box " << std::endl;
      newMesh->prepareVertexBuffer();
      std::cout << "\tPrepared vertex buffer " << std::endl;
    return;
  }
  std::cout << "Generating LOD with cell size: "<< cellSize<< std::endl;
  //Copy the mesh
  
  for(unsigned int i = 0; i < mesh->faces_.size(); ++i)
  {
    newMesh->faces_.push_back(mesh->faces_[i]);
  }
  ///////

  float sizeBoundBox_x = mesh->max_[0] - mesh->min_[0];
  float sizeBoundBox_y = mesh->max_[1] - mesh->min_[1];
  float sizeBoundBox_z = mesh->max_[2] - mesh->min_[2];
  int nCellsX = ceil(sizeBoundBox_x/cellSize);
  int nCellsY = ceil(sizeBoundBox_y/cellSize);
  int nCellsZ = ceil(sizeBoundBox_z/cellSize);
  std::cout << "\t NumCells X Y Z: "<< nCellsX << " "  << nCellsY<< " " << nCellsZ << std::endl;
  if(nCellsX < 1 || nCellsY < 1 || nCellsZ < 1 )
  {
    std::cerr << "\tGrid too small to generate LOD" << std::endl;
    exit(0);
  }
  //XYZ mat with vertices index
  std::vector<std::vector<std::vector<std::vector<int> > > > cells(nCellsX, std::vector<std::vector<std::vector<int> > >(nCellsY,std::vector<std::vector<int> >(nCellsZ,std::vector<int>(0))));
  std::vector<std::vector<std::vector<std::vector<int> > > > cellsExtraFaces(nCellsX, std::vector<std::vector<std::vector<int> > >(nCellsY,std::vector<std::vector<int> >(nCellsZ,std::vector<int>(0))));
  std::vector<bool> pushed_vertices(mesh->vertices_.size()/3,false);
  std::cout << "\t Space allocated, numVertices = " << pushed_vertices.size()<< std::endl;
  for(unsigned int i =0; i < mesh->faces_.size(); i = i +3)
  {
    //std::cout << "\t\t ." << std::endl;
    for(int j = 0; j < 3; ++j)
    {
      
      int indexVertex = mesh->faces_[i+j];
      //std::cout << "\t\t\t ." << indexVertex << std::endl;
      if(!pushed_vertices[indexVertex])
      {
        int cellX = std::min((int)floor((mesh->vertices_[indexVertex*3] - mesh->min_[0])/cellSize),nCellsX-1);
        int cellY = std::min((int)floor((mesh->vertices_[indexVertex*3+1] - mesh->min_[1])/cellSize),nCellsY-1);
        int cellZ = std::min((int)floor((mesh->vertices_[indexVertex*3+2] - mesh->min_[2])/cellSize),nCellsZ-1);
        //std::cout << "\t\t\t Pos " << mesh->vertices_[indexVertex] << " " << mesh->vertices_[indexVertex+1] << " " << mesh->vertices_[indexVertex+2] << std::endl;
        //std::cout << "\t\t\t Pushing Cell " << cellX << " " << cellY << " " << cellZ << std::endl;
        cells[cellX][cellY][cellZ].push_back(i+j);
        //std::cout << "\t\t\t Pushed Cell " << cellX << " " << cellY << " " << cellZ << std::endl;
        pushed_vertices[indexVertex] = true;
      }
      else 
      {
        int cellX = std::min((int)floor((mesh->vertices_[indexVertex*3] - mesh->min_[0])/cellSize),nCellsX-1);
        int cellY = std::min((int)floor((mesh->vertices_[indexVertex*3+1] - mesh->min_[1])/cellSize),nCellsY-1);
        int cellZ = std::min((int)floor((mesh->vertices_[indexVertex*3+2] - mesh->min_[2])/cellSize),nCellsZ-1);
        cellsExtraFaces[cellX][cellY][cellZ].push_back(i+j);
      }
    }
  }
  bool trobat = false;
  for(unsigned int i=0; i < pushed_vertices.size(); ++i)
  {
    if(!pushed_vertices[i]) trobat = true;
  }
  if(trobat){
    //std::cerr << "\tERROR" << std::endl;
  }
  std::cout << "\t Grid done" << std::endl;
  int newIndexVertex = 0;
  newMesh->vertices_.resize(static_cast<size_t>(nCellsX*nCellsY*nCellsZ) * 3);

  for(int i =0; i < nCellsX; ++i)
  {
    for(int j =0; j < nCellsY; ++j)
    {
      for(int k =0; k < nCellsZ; ++k)
      {
        float newVertex_x =0;
        float newVertex_y =0;
        float newVertex_z =0;
        int faceVertexIndex = -1;
        int nVerticesInCell = cells[i][j][k].size();
        for(int v = 0; v < nVerticesInCell; ++v)
        {
          faceVertexIndex = cells[i][j][k][v];
          int vertexIndex = newMesh->faces_[faceVertexIndex];

          newMesh->faces_[faceVertexIndex] = newIndexVertex;
          //std::cout << "\t Replacing " << std::endl;
          newVertex_x += mesh->vertices_[vertexIndex*3];
          newVertex_y += mesh->vertices_[vertexIndex*3+1];
          newVertex_z += mesh->vertices_[vertexIndex*3+2];
        }
        newVertex_x = newVertex_x/nVerticesInCell;
        newVertex_y = newVertex_y/nVerticesInCell;
        newVertex_z = newVertex_z/nVerticesInCell;
        nVerticesInCell = cellsExtraFaces[i][j][k].size();
        for(int v = 0; v < nVerticesInCell; ++v)
        {
          faceVertexIndex = cellsExtraFaces[i][j][k][v];
          //int vertexIndex = newMesh->faces_[faceVertexIndex];

          newMesh->faces_[faceVertexIndex] = newIndexVertex;
          //std::cout << "\t Replacing " << std::endl;
        }

        
        newMesh->vertices_[newIndexVertex*3] = newVertex_x;
        newMesh->vertices_[newIndexVertex*3+1] =newVertex_y;
        newMesh->vertices_[newIndexVertex*3+2] = newVertex_z;
        if(cells[i][j][k].size()>0)
        {

          newIndexVertex +=1;
        }
      }
    }
  }
  std::cout << "\t Collapse done " <<  std::endl;
  //Remove extra faces
  int auxIndex =0;

  for(auto it = newMesh->faces_.begin(); it != newMesh->faces_.end(); )
  {
    if(*(it) == *(it+1) || *(it) == *(it+2) || *(it+1) == *(it+2))
    {
      it = newMesh->faces_.erase(it,it+3);
      //std::cout << newMesh->faces_.size() << " FaceErased " << auxIndex  << " " << newMesh->faces_.size() <<" \t" << std::endl;
    } 
    else 
    {
      it = it+3;
    }
    auxIndex +=1;
    //std::cout << newMesh->faces_.size() << " " << auxIndex <<" \t" << std::endl;
    //if((auxIndex*100/newMesh->faces_.size())%5==0) std::cout << (auxIndex*100/newMesh->faces_.size()) <<"% \t" << std::endl;
  }
  
  /*
  std::cout << "\t extra faces removed" << std::endl;
  int c = 0;
  for(int i=0; i < newMesh->faces_.size(); ++i)
  {
    std::cout << "\t" << newMesh->faces_[i] << " ";

    c++;
    if(c ==3) {
      std::cout << std::endl;
      c = 0;
    }
  }*/
  std::cout << "\t totalVertices " << newIndexVertex << " " << newMesh->vertices_.size() << " " << mesh->vertices_.size()/3<< std::endl;
  ComputeVertexNormals(newMesh->vertices_, newMesh->faces_, &newMesh->normals_);
  std::cout << "\tGenerated normals " << std::endl;
  ComputeBoundingBox(newMesh->vertices_, newMesh);
  std::cout << "\tGenerated bounding box " << std::endl;
  newMesh->prepareVertexBuffer();
  std::cout << "\tPrepared vertex buffer " << std::endl;


}



bool WriteToPly(const std::string &filename, const TriangleMesh &mesh) {
  (void)filename;
  (void)mesh;

  std::cerr << "Not yet implemented" << std::endl;

  // TODO(students): Implement storing to PLY format.

  // END.

  return false;
}

}  // namespace data_representation
