#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QLabel>
#include <QSlider>
#include <QSplitter>
#include <QGraphicsView>
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
#include <QBarSet>
#include <QBarSeries>
#include <QValueAxis>

#include "imageprocessor.h"

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
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void loadImage();
    void testLoadImage();
    void applyTransformations();

private:
    void setupUI();
    void updateImage();
    void resetImage();
    void calculateHistogram();

private:
    Ui::MainWindow *ui;

    QGraphicsView *m_imageView;
    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_imageItem;
    QImage m_originalImage;
    QImage m_processedImage;

    QTimer *m_updateTimer;
    QSlider *m_brightnessSlider;
    QLabel *m_brightnessLabel;
    QSlider *m_contrastSlider;
    QLabel *m_contrastLabel;
    QCheckBox *m_grayscaleCheckBox;
    QCheckBox *m_negativeCheckBox;
    QComboBox *m_binariztionMethod;
    QSlider *m_thresholdSlider;

    QGraphicsRectItem *m_selectionItem = nullptr;
    QPointF m_startPoint;
    QRect m_selectionRect;
    bool m_isSelecting = false;

    QChart *m_chart;
    QChartView *m_chartView;
};
#endif // MAINWINDOW_H
