#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "imageprocessor.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_scale(1.0f)
{
    ui->setupUi(this);

    this->setupUI();

    m_scene->clear();
    m_imageItem = nullptr;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI() {
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(splitter);

    // Image Viewer
    m_imageView = new QGraphicsView();
    m_imageView->setDragMode(QGraphicsView::ScrollHandDrag);
    m_imageView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_imageView->setAlignment(Qt::AlignCenter);
    splitter->addWidget(m_imageView);

    m_scene = new QGraphicsScene(this);
    m_imageView->setScene(m_scene);

    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    splitter->addWidget(rightPanel);

    // Control
    QGroupBox *ctrlBox = new QGroupBox("Управление изображением");
    QHBoxLayout *ctrlLayout = new QHBoxLayout(ctrlBox);

    QPushButton *loadImageButton = new QPushButton("Загрузить изображение");
    connect(loadImageButton, &QPushButton::clicked, this, &MainWindow::loadImage);
    QPushButton *resetButton = new QPushButton("Сбросить");
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetImage);

    ctrlLayout->addWidget(loadImageButton);
    ctrlLayout->addWidget(resetButton);
    rightLayout->addWidget(ctrlBox);

    // Scaling
    QGroupBox *scalingGroup = new QGroupBox("Масштабирование");
    QVBoxLayout *scalingLayout = new QVBoxLayout(scalingGroup);

    QHBoxLayout *scalingRow1Layout = new QHBoxLayout();

    QPushButton *scaling1Button = new QPushButton("x0.25");
    connect(scaling1Button, &QPushButton::clicked, this, [this](){
        m_scale = 0.25f;
        m_scalingValueInput->setText("0.25");
        scaleImage();
    });
    QPushButton *scaling2Button = new QPushButton("x0.5");
    connect(scaling2Button, &QPushButton::clicked, this, [this](){
        m_scale = 0.5f;
        m_scalingValueInput->setText("0.5");
        scaleImage();
    });

    scalingRow1Layout->addWidget(scaling1Button);
    scalingRow1Layout->addWidget(scaling2Button);

    QHBoxLayout *scalingRow2Layout = new QHBoxLayout();

    QPushButton *scaling3Button = new QPushButton("x2");
    connect(scaling3Button, &QPushButton::clicked, this, [this](){
        m_scale = 2.0f;
        m_scalingValueInput->setText("2.0");
        scaleImage();
    });
    QPushButton *scaling4Button = new QPushButton("x4");
    connect(scaling4Button, &QPushButton::clicked, this, [this](){
        m_scale = 4.0f;
        m_scalingValueInput->setText("4.0");
        scaleImage();
    });

    scalingRow2Layout->addWidget(scaling3Button);
    scalingRow2Layout->addWidget(scaling4Button);

    m_scalingValueInput = new QLineEdit();
    m_scalingValueInput->setText("1.0");

    m_interpolationMethod = new QComboBox();
    m_interpolationMethod->addItems({"Метод ближайшего соседа", "Билинейная интерполяция"});

    QPushButton *applyScalingButton = new QPushButton("Применить масштабирование");
    connect(applyScalingButton, &QPushButton::clicked, this, [this](){
        bool ok;
        float newScale = m_scalingValueInput->text().toFloat(&ok);

        if (!ok || newScale <= 0.0f) {
            QMessageBox::warning(this, "Ошибка", "Некорректное значение, введите число больше 0.");
            return;
        }

        m_scale = newScale;
        scaleImage();
    });

    scalingLayout->addLayout(scalingRow1Layout);
    scalingLayout->addLayout(scalingRow2Layout);
    scalingLayout->addWidget(new QLabel("Коэффициент:"));
    scalingLayout->addWidget(m_scalingValueInput);
    scalingLayout->addWidget(new QLabel("Метод интерполяции:"));
    scalingLayout->addWidget(m_interpolationMethod);
    scalingLayout->addWidget(applyScalingButton);
    rightLayout->addWidget(scalingGroup);

    rightLayout->addStretch(); // Push widgets to top
    splitter->setStretchFactor(0, 1); // Image takes available space
    splitter->setStretchFactor(1, 0); // Controls take minimum space
}

void MainWindow::loadImage() {
    QString fileName = QFileDialog::getOpenFileName(
        this, "Открыть изображение", "", "Изображения (*.png *.jpg *.jpeg *.bmp)"
        );

    if (fileName.isEmpty()) return;

    if (!m_originalImage.load(fileName)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось загрузить изображение.");
        return;
    }

    m_originalImage = m_originalImage.convertToFormat(QImage::Format_ARGB32);
    m_processedImage = m_originalImage;

    m_scene->clear();
    m_imageItem = nullptr;

    updateImage();
}

void MainWindow::updateImage() {
    if (m_processedImage.isNull()) return;

    QPixmap pixmap = QPixmap::fromImage(m_processedImage);

    // If the item doesn't exist yet, create it
    if (!m_imageItem) {
        m_scene->clear();
        m_imageItem = m_scene->addPixmap(pixmap);
    } else {
        m_imageItem->setPixmap(pixmap);
    }

    m_scene->setSceneRect(pixmap.rect());
    m_imageView->resetTransform();
    m_imageView->centerOn(m_imageItem);
}

void MainWindow::resetImage() {
    if (m_originalImage.isNull()) return;

    m_processedImage = m_originalImage;
    updateImage();
}

void MainWindow::scaleImage() {
    int method = m_interpolationMethod->currentIndex();

    if (method == 0) { // Nearest neighbour
        m_processedImage = ImageProcessor::applyNearestNeighbourScaling(m_originalImage, m_scale);
    } else { // Bilinear interpolation
        m_processedImage = ImageProcessor::applyBilinearScaling(m_originalImage, m_scale);
    }

    updateImage();
}













