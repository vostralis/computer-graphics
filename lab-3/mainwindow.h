#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QTimer>

#include "scene.h"

#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>
#include <QSlider>

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
    void onLightingModelChanged();

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

    QRadioButton *simpleModel;
    QRadioButton *gouraudShading;

    QSlider *resolutionSlider;
    QLabel *resolutionLabel;

    QLabel *ambientLightIntensityLabel;
    QSlider *ambientLightIntensitySlider;
    QLabel *diffuseReflectionCoefficientLabel;
    QSlider *diffuseReflectionCoefficientSlider;
    QLabel *ambientReflectionCoefficientLabel;
    QSlider *ambientReflectionCoefficientSlider;
    QLabel *lightSourceIntensityLabel;
    QSlider *lightSourceIntensitySlider;
};
#endif // MAINWINDOW_H
