#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QTimer>
#include <QLabel>
#include <QRadioButton>

#include "scene.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateLabels(QVector3D position, float rotationX, float rotationY, float rotationZ, float scale);
    void onRenderModeChanged();

private:
    Ui::MainWindow *ui;

    QWidget *mainContainer;
    QHBoxLayout *mainLayout;
    QVBoxLayout *sidebarLayout;

    QVBoxLayout *sceneLayout;
    Scene *sceneWidget;
    QTimer *timer;

    QLabel *tLabel;
    QLabel *rLabel;
    QLabel *sLabel;

    QRadioButton *withoutCullingButton;
    QRadioButton *withCullingButton;
    QRadioButton *visulizationButton;
};
#endif // MAINWINDOW_H
