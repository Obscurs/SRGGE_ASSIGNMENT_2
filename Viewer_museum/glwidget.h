// Author: Marc Comino 2020

#ifndef GLWIDGET_H_
#define GLWIDGET_H_

#include <GL/glew.h>
#include <QGLWidget>
#include <QImage>
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QString>

#include <memory>

#include "./camera.h"
#include "./triangle_mesh.h"
static const unsigned int SIZEMAP = 30;
static const unsigned int N_LODS = 4;
//static const unsigned int MAX_TRIS = 35000000;
static const unsigned int MAX_TRIS = 15000000;
static const unsigned int RAYS_PER_CELL = 500;
class GLWidget : public QGLWidget {
  Q_OBJECT

 public:
  explicit GLWidget(QWidget *parent = nullptr);
  ~GLWidget();

  /**
   * @brief LoadModel Loads a PLY model at the filename path into the mesh_ data
   * structure.
   * @param filename Path to the PLY model.
   * @return Whether it was able to load the model.
   */
  bool LoadModel(const QString &filename);
  void GenTranslations();

    void update();
 protected:
  /**
   * @brief initializeGL Initializes OpenGL variables and loads, compiles and
   * links shaders.
   */
  void initializeGL();

  /**
   * @brief resizeGL Resizes the viewport.
   * @param w New viewport width.
   * @param h New viewport height.
   */


  
  void resizeGL(int w, int h);


  void timer_start(unsigned int interval);
  void reloadShaders();
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void keyPressEvent(QKeyEvent *event);

 private:
  /**
   * @brief program_ The reflection shader program.
   */
  std::unique_ptr<QOpenGLShaderProgram> shader_program_;
  std::unique_ptr<QOpenGLShaderProgram> wall_program_;



  /**
   * @brief camera_ Class that computes the multiple camera transform matrices.
   */
  data_visualization::Camera camera_;

  /**
   * @brief mesh_ Data structure representing a triangle mesh.
   */
  std::unique_ptr<data_representation::TriangleMesh> mesh_[(N_LODS+1)];

std::vector<int> indexCoordsLOD[(N_LODS+1)];


GLuint modelVAO[(N_LODS+1)];
GLuint modelVBO[(N_LODS+1)];
GLuint modelEBO[(N_LODS+1)];

GLuint wallVAO;
GLuint wallVBO;
GLuint wallEBO;

  int last_time_;
  int aux_timer_;

  bool initialized_;

  glm::vec3 last_camera_pos_;
  /**
   * @brief width_ Viewport current width.
   */
  float width_;

  /**
   * @brief height_ Viewport current height.
   */
  float height_;


  int mode_;

  unsigned int currentLod_;

  void RecalculateIndicesLOD(const glm::vec3 &cameraPos, glm::vec3* translations, int n_instances);

 protected slots:
  /**
   * @brief paintGL Function that handles rendering the scene.
   */
  void paintGL();


  void SetQuadratics(bool set);

  void SetOctree(bool set);

  void SetAverage(bool set);

  void SetLod1(bool set);
  void SetLod2(bool set);
  void SetLod3(bool set);
  void SetLod4(bool set);
  void SetLod5(bool set);




 signals:
  /**
   * @brief SetFaces Signal that updates the interface label "Faces".
   */
  void SetFaces(QString);

  /**
   * @brief SetFaces Signal that updates the interface label "Vertices".
   */
  void SetVertices(QString);

  /**
   * @brief SetFaces Signal that updates the interface label "Framerate".
   */
  void SetFramerate(QString);
};

#endif  //  GLWIDGET_H_
