// Author: Marc Comino 2020

#include <triangle_mesh.h>

#include <algorithm>
#include <limits>
#include <math.h>       /* sqrt */
namespace data_representation {

TriangleMesh::TriangleMesh() { Clear(); }

void TriangleMesh::Clear() {
  vertices_.clear();
  faces_.clear();
  normals_.clear();
  buffer_.clear();

  min_ = Eigen::Vector3f(std::numeric_limits<float>::max(),
                         std::numeric_limits<float>::max(),
                         std::numeric_limits<float>::max());
  max_ = Eigen::Vector3f(std::numeric_limits<float>::lowest(),
                         std::numeric_limits<float>::lowest(),
                         std::numeric_limits<float>::lowest());
}

void TriangleMesh::prepareVertexBuffer()
{
    buffer_.clear();
    for(unsigned int i=0; i < vertices_.size()/3;++i)
    {
        for(int j=0; j< 3; ++j)
        {
            buffer_.push_back(vertices_[i*3+j]);
        }
        for(int j=0; j< 3; ++j)
        {
            buffer_.push_back(normals_[i*3+j]);
        }
    }
}
float TriangleMesh::getContribution(const glm::vec3 &posMode, const glm::vec3 &posCamera)
{
  float d = sqrt((max_[0]- min_[0])*(max_[0]- min_[0]) + (max_[1]- min_[1])*(max_[1]- min_[1]) + (max_[2]- min_[2])*(max_[2]- min_[2]));
  float D = glm::distance(posMode,posCamera);
  return d/(D*faces_.size()/3+0.000000000001);
}
}  // namespace data_representation
