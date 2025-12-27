#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QSplitter>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QFormLayout>
#include <QImage>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QLabel>
#include <QCheckBox>

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

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void loadImage();
    void applyNoiseFilter();
    void applySharpeningFilter();

private:
    void setupUI();
    void updateImage();
    void resetImage();

private:
    Ui::MainWindow *ui;

    QGraphicsView *m_imageView;
    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_imageItem;
    QImage m_originalImage;
    QImage m_processedImage;

    QTimer *m_updateTimer;
    QSlider *m_noiseIntensitySlider;
    QSlider *m_apertureSlider;
    QComboBox *m_noiseReductionBox;
    QComboBox *m_sharpeningMethodBox;
    QSlider *m_sharpeningSlider;
};
#endif // MAINWINDOW_H
