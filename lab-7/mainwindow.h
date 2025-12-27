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
#include <QPushButton>
#include <QImage>
#include <QFileDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QComboBox>

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

private:
    void setupUI();
    void updateImage();
    void resetImage();
    void loadImage();
    void scaleImage();

private:
    Ui::MainWindow *ui;

    QGraphicsView *m_imageView;
    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_imageItem;
    QImage m_originalImage;
    QImage m_processedImage;

    float m_scale;
    QComboBox *m_interpolationMethod;
    QLineEdit *m_scalingValueInput;
};
#endif // MAINWINDOW_H
