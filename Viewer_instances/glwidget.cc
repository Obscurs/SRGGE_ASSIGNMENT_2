// Author: Marc Comino 2020
#define STB_IMAGE_IMPLEMENTATION
#include <glwidget.h>
#include <stb_image.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <chrono>
#include <ctime>  
#include "./mesh_io.h"
#include "./triangle_mesh.h"
#include <thread>
#include <functional>
#include <ctime>  
namespace {

const double kFieldOfView = 60;
const double kZNear = 0.0001;
const double kZFar = 20;

const char kNormalVertexShaderFile[] = "../shaders/normal.vert";
const char kNormalFragmentShaderFile[] = "../shaders/normal.frag";

const int kVertexAttributeIdx = 0;
const int kNormalAttributeIdx = 1;


// pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
// ----------------------------------------------------------------------------------------------

const int N_INSTANCES = 8000;
glm::vec3 translations[N_INSTANCES];


bool ReadFile(const std::string filename, std::string *shader_source) {
  std::ifstream infile(filename.c_str());

  if (!infile.is_open() || !infile.good()) {
    std::cerr << "Error " + filename + " not found." << std::endl;
    return false;
  }

  std::stringstream stream;
  stream << infile.rdbuf();
  infile.close();

  *shader_source = stream.str();
  return true;
}




bool LoadProgram(const std::string &vertex, const std::string &fragment,
                 QOpenGLShaderProgram *program) {
  std::string vertex_shader, fragment_shader;
  bool res =
      ReadFile(vertex, &vertex_shader) && ReadFile(fragment, &fragment_shader);

  if (res) {
    program->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                     vertex_shader.c_str());
    program->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                     fragment_shader.c_str());
    program->bindAttributeLocation("vertex", kVertexAttributeIdx);
    program->bindAttributeLocation("normal", kNormalAttributeIdx);
    program->link();
    std::cerr << "Program loaded: " + vertex + "   " + fragment << std::endl;
  }
  else {
      std::cerr << "ERROR LOADING: " + vertex + "   " + fragment << std::endl;
  }
  return res;
}

}  // namespace

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent),
      initialized_(false),
      width_(0.0),
      height_(0.0),
      mode_(0),
      currentLod_(0),
      aux_timer_(0),
      last_time_(0),
      recalculate_(false) {
  setFocusPolicy(Qt::StrongFocus);
}

GLWidget::~GLWidget() {
  if (initialized_) {

  }
}/*
void GLWidget::timer_start(unsigned int interval)
{
    std::thread([this, interval]() {
        while (true)
        {
            update();
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
    }).detach();
}*/

bool GLWidget::LoadModel(const QString &filename) {
  //Intances positions
  int index = 0;
  float offset = 0.f;
  for(int y = 0; y < 20; y += 1)
  {
      for(int x = 0; x < 20; x += 1)
      {
        for(int z = 0; z < 20; z +=1)
        {

            glm::vec3 translation;
            translation.x = (float)x / 4.f + offset;
            translation.y = (float)y / 4.f + offset/2;
            translation.z = (float)z / 4.f + offset/2;
            translations[index++] = translation;
        }
          
      }
  } 


  std::string file = filename.toUtf8().constData();
  size_t pos = file.find_last_of(".");
  std::string type = file.substr(pos + 1);

  std::unique_ptr<data_representation::TriangleMesh> mesh = std::make_unique<data_representation::TriangleMesh>();
  

  bool res = false;
  if (type.compare("ply") == 0) {
    res = data_representation::ReadFromPly(file, mesh.get());
  }
  std::cout << "..................." << std::endl;
  if (res) {


      for(int i= 0; i<=N_LODS; i++)
      {
        std::cout << "Iteration " <<  i << std::endl;
        std::unique_ptr<data_representation::TriangleMesh> meshLOD = std::make_unique<data_representation::TriangleMesh>();
        //data_representation::GenLodMeshQuadratics(0.25, mesh.get(),meshLOD.get());

        data_representation::GenLodMeshAverage(0.0025*i, mesh.get(),meshLOD.get());

        //mesh_.reset(mesh.release());
        mesh_[i].reset(meshLOD.release());
      
        
        std::cout << "Model has " <<  mesh_[i]->buffer_.size() << " elements in buffer" << std::endl;
        // TODO(students): Create / Initialize buffers.
        glGenVertexArrays(1, &modelVAO[i]);
        glGenBuffers(1, &modelVBO[i]);
        glGenBuffers(1, &modelEBO[i]);
        glBindVertexArray(modelVAO[i]);
        glBindBuffer(GL_ARRAY_BUFFER, modelVBO[i]);
        glBufferData(GL_ARRAY_BUFFER, mesh_[i]->buffer_.size()* sizeof(float), &mesh_[i]->buffer_[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelEBO[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_[i]->faces_.size() * sizeof(int), &mesh_[i]->faces_[0], GL_STATIC_DRAW);

      }
    
    //data_representation::Node::deleteOctree(nodes);

    //emit SetFaces(QString(std::to_string(mesh_->faces_.size() / 3).c_str()));
    //emit SetVertices(
    //    QString(std::to_string(mesh_->vertices_.size() / 3).c_str()));
    std::cerr << "Model loaded " + file << std::endl;
    return true;
  }
  std::cerr << "ERROR loading model " + file << std::endl;
  return false;
}



void GLWidget::initializeGL() {
  glewExperimental=true;

  glewInit();

  glEnable(GL_NORMALIZE);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);



  shader_program_ = std::make_unique<QOpenGLShaderProgram>();

  bool res =
      LoadProgram(kNormalVertexShaderFile, kNormalFragmentShaderFile,
                  shader_program_.get());
  
  if (!res) exit(0);
  std::cerr << "All programs loaded" << std::endl;

 
  initialized_ = true;
  std::cerr << "Gl init OK" << std::endl;

  glDepthFunc(GL_LEQUAL);
  // enable seamless cubemap sampling for lower mip levels in the pre-filter map.
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);


  LoadModel("../models/bunny.ply");
  std::cerr << "Default model loaded OK" << std::endl;
  //timer_start(1000);
}

void GLWidget::reloadShaders()
{
    shader_program_.reset();
    shader_program_ = std::make_unique<QOpenGLShaderProgram>();
    LoadProgram(kNormalVertexShaderFile, kNormalFragmentShaderFile,
                shader_program_.get());


}

void GLWidget::resizeGL(int w, int h) {
  if (h == 0) h = 1;
  width_ = w;
  height_ = h;

  camera_.SetViewport(0, 0, w, h);
  camera_.SetProjection(kFieldOfView, kZNear, kZFar);
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    camera_.StartRotating(event->x(), event->y());
  }
  if (event->button() == Qt::RightButton) {
    camera_.StartZooming(event->x(), event->y());
  }
  update();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
  camera_.SetRotationX(event->y());
  camera_.SetRotationY(event->x());
  camera_.SafeZoom(event->y());
  update();
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    camera_.StopRotating(event->x(), event->y());
  }
  if (event->button() == Qt::RightButton) {
    camera_.StopZooming(event->x(), event->y());
  }
  update();
}

void GLWidget::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Up) camera_.Zoom(-1);
  if (event->key() == Qt::Key_Down) camera_.Zoom(1);

  if (event->key() == Qt::Key_Left) camera_.Rotate(-1);
  if (event->key() == Qt::Key_Right) camera_.Rotate(1);

  if (event->key() == Qt::Key_W) camera_.Zoom(-1);
  if (event->key() == Qt::Key_S) camera_.Zoom(1);

  if (event->key() == Qt::Key_A) camera_.Rotate(-1);
  if (event->key() == Qt::Key_D) camera_.Rotate(1);

  if (event->key() == Qt::Key_R) {
    reloadShaders();
  }

  update();
}

void GLWidget::paintGL() {
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (initialized_) {
    //camera_.UpdateModel(mesh_[currentLod_]->min_, mesh_[currentLod_]->max_);
    camera_.SetViewport();

    Eigen::Matrix4f projection = camera_.SetProjection();
    Eigen::Matrix4f view = camera_.SetView();
    Eigen::Matrix4f model = camera_.SetModel();
    Eigen::Matrix4f t2 = view;
    t2 = t2.inverse().transpose();
    glm::vec3 cameraPos(t2(3,0), t2(3,1), t2(3,2));
    if(cameraPos != last_camera_pos_ || recalculate_) RecalculateIndicesLOD(cameraPos,&translations[0],N_INSTANCES);

   /* Eigen::Matrix4f t = view* model;
    Eigen::Matrix3f normal;
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j) normal(i, j) = t(i, j);

    normal = normal.inverse().transpose();
*/
    //std::cerr << "Painting lod " << currentLod_ << " mode " << mode_<< std::endl;

    int numFaces =0;
    for(int i=0; i < N_LODS+1; ++i)
    {
      if (mesh_[i] != nullptr && indexCoordsLOD[i].size() > 0) {
        GLint projection_location, view_location, model_location;


        shader_program_->bind();
        projection_location = shader_program_->uniformLocation("projection");
        view_location = shader_program_->uniformLocation("view");
        model_location = shader_program_->uniformLocation("model");

        glUniformMatrix4fv(projection_location, 1, GL_FALSE, projection.data());
        glUniformMatrix4fv(view_location, 1, GL_FALSE, view.data());
        glUniformMatrix4fv(model_location, 1, GL_FALSE, model.data());

        int currentInstance =0;
        int firstInstance =0;
        int numInstances = 0;
        while(currentInstance < indexCoordsLOD[i].size())
        {
          GLint offsets_location;
          std::string name = "offsets[" + std::to_string(currentInstance-firstInstance) + "]";
          offsets_location = shader_program_->uniformLocation(name.c_str());
          glUniform3fv(offsets_location, 1, &translations[indexCoordsLOD[i][currentInstance]][0]);
          numInstances++;
          currentInstance++;
          if(numInstances == 1000 || currentInstance == indexCoordsLOD[i].size())
          {
            glBindVertexArray(modelVAO[i]);
            glDrawElementsInstanced(GL_TRIANGLES, mesh_[i]->faces_.size(), GL_UNSIGNED_INT, 0,numInstances);
            glBindVertexArray(0);
            numInstances =0;
            firstInstance = currentInstance;
          } 
        }



        numFaces += mesh_[i]->faces_.size()*indexCoordsLOD[i].size() / 3;
        
        //emit SetVertices(QString(std::to_string(mesh_[i]->vertices_.size()*indexCoordsLOD[i].size() / 3).c_str()));

        //glBindVertexArray(modelVAO[currentLod_]);
        ////glDrawElements(GL_TRIANGLES, mesh_->faces_.size(), GL_UNSIGNED_INT, 0);
        //glDrawElementsInstanced(GL_TRIANGLES, mesh_[currentLod_]->faces_.size(), GL_UNSIGNED_INT, 0, N_INSTANCES);

        ////glDrawArraysInstanced(GL_TRIANGLES, 0, mesh_->faces_.size(), N_INSTANCES); 

        //glBindVertexArray(0);




      }
    }
    emit SetFaces(QString(std::to_string(numFaces).c_str()));
    last_camera_pos_ = cameraPos;
  }
}

void GLWidget::RecalculateIndicesLOD(const glm::vec3 &cameraPos, glm::vec3* translations, int n_instances)
{
  recalculate_ = false;
  unsigned int currentNumFaces =0;
  unsigned int maxNumFaces = MAX_TRIS;
  std::vector<float> contributions[N_LODS+1];
  std::vector<int> currLODtrans(n_instances, N_LODS);
  for(int i=0; i< N_LODS+1;++i)
  {
    indexCoordsLOD[i].clear();
    for(int j=0; j<n_instances; ++j)
    {
      float contrib = mesh_[i]->getContribution(translations[j],cameraPos);
      contributions[i].push_back(contrib);
    }
  }
  for(int i=0; i< n_instances; ++i)
  {
    //indexCoordsLOD[N_LODS].push_back(i);
    currentNumFaces += mesh_[N_LODS]->faces_.size()/3;
  }
  bool end = false;
  std::vector<float> contributionsDiference(n_instances, 0);
  while(!end)
  {
    //std::cout << "Not end" << std::endl;
    float biggest_difference = 0;
    int index_biggest =0;
    for(int i=0; i<n_instances; ++i)
    {

      float curr = contributions[currLODtrans[i]][i];
      float next = curr;

      if(currLODtrans[i]>0) next = contributions[currLODtrans[i]-1][i];
      contributionsDiference[i] = curr - next;
      //std::cout << "iter: " << i << " LOD "<< currLODtrans[i] << " " <<curr << " " << next<< std::endl;
      int auxCurrFaces = mesh_[currLODtrans[i]]->faces_.size()/3;
      if(contributionsDiference[i] > biggest_difference) 
      {
        int auxNextFaces = mesh_[currLODtrans[i]-1]->faces_.size()/3;
        if(auxNextFaces-auxCurrFaces + currentNumFaces < maxNumFaces)
        {
          biggest_difference = contributionsDiference[i];
          index_biggest = i;
        }
      }
    }
    if(biggest_difference == 0) 
      end = true;
    else
    {
      int auxCurrFaces = mesh_[currLODtrans[index_biggest]]->faces_.size()/3;
      int auxNextFaces = mesh_[currLODtrans[index_biggest]-1]->faces_.size()/3;
      currLODtrans[index_biggest] -=1;
      currentNumFaces += auxNextFaces - auxCurrFaces;
    }
    //std::cout << "curr tris: " << currentNumFaces << std::endl;
  }
  for(int i=0; i< n_instances;++i)
  {
    indexCoordsLOD[currLODtrans[i]].push_back(i);
  }
  /*for(int i=0; i< N_LODS+1;++i)
  {
    std::cout << indexCoordsLOD[i].size() << std::endl;
  }*/

  emit SetLod1Inst(QString(std::to_string(indexCoordsLOD[0].size()).c_str()));
  emit SetLod2Inst(QString(std::to_string(indexCoordsLOD[1].size()).c_str()));
  emit SetLod3Inst(QString(std::to_string(indexCoordsLOD[2].size()).c_str()));
  emit SetLod4Inst(QString(std::to_string(indexCoordsLOD[3].size()).c_str()));
  emit SetLod5Inst(QString(std::to_string(indexCoordsLOD[4].size()).c_str()));

}



void GLWidget::SetMaxTriangles(int max)
{
  recalculate_ = true;
  MAX_TRIS = max*1000000;
  update();
}

void GLWidget::update()
{
  if(initialized_)
  {

      std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
      auto duration = now.time_since_epoch();
      auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

      //std::chrono::duration<double> now_aux = now;
      updateGL();
      unsigned int elapsed_millis = (millis-last_time_);
      aux_timer_ += elapsed_millis;
      if(aux_timer_ <0) aux_timer_ = 0;
      last_time_ = millis;
      //std::time_t end_time = std::chrono::system_clock::to_time_t(end);
      if(aux_timer_ > 350)
      {
        aux_timer_ = 0;
        emit SetFramerate(QString(std::to_string(1000/elapsed_millis).c_str()));
      }
      
  }

}