// Author: Marc Comino 2020

#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <QMainWindow>
#include <QTimer>
namespace Ui {
class MainWindow;
}

namespace gui {

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  virtual void show();

 private slots:
  /**
   * @brief on_actionQuit_triggered Closes the application.
   */
  void on_actionQuit_triggered();



  void updateTimer();

 private:
  Ui::MainWindow *ui;
    QTimer *timer;
};

}  //  namespace gui

#endif  //  MAIN_WINDOW_H_
