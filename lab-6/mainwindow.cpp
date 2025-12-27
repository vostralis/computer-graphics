#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "imageprocessor.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
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


    // Noise
    QGroupBox *noiseGroup = new QGroupBox("Наложение шума");
    QVBoxLayout *noiseLayout = new QVBoxLayout(noiseGroup);

    QHBoxLayout *noiseSelectionLayout = new QHBoxLayout();

    QPushButton *dotButton = new QPushButton("Точки");
    connect(dotButton, &QPushButton::clicked, this, [this](){
        m_processedImage = ImageProcessor::applySpotNoise(m_processedImage, m_noiseIntensitySlider->value() * 100);
        updateImage();
    });
    noiseSelectionLayout->addWidget(dotButton);

    QPushButton *lineButton = new QPushButton("Линии");
    connect(lineButton, &QPushButton::clicked, this, [this](){
        m_processedImage = ImageProcessor::applyLineNoise(m_processedImage, m_noiseIntensitySlider->value() * 2);
        updateImage();
    });
    noiseSelectionLayout->addWidget(lineButton);

    QPushButton *circleButton = new QPushButton("Окружности");
    connect(circleButton, &QPushButton::clicked, this, [this](){
        m_processedImage = ImageProcessor::applyCircleNoise(m_processedImage, m_noiseIntensitySlider->value() * 10);
        updateImage();
    });
    noiseSelectionLayout->addWidget(circleButton);

    QHBoxLayout *noiseIntensityLayout = new QHBoxLayout();
    QLabel *noiseIntensityValue = new QLabel("10");

    m_noiseIntensitySlider = new QSlider(Qt::Horizontal);
    m_noiseIntensitySlider->setRange(1, 100);
    m_noiseIntensitySlider->setValue(10);
    connect(m_noiseIntensitySlider, &QSlider::valueChanged,  this, [this, noiseIntensityValue](){
        noiseIntensityValue->setText(QString("%1").arg(m_noiseIntensitySlider->value()));
    });

    noiseIntensityLayout->addWidget(new QLabel("Интенсивность: "));
    noiseIntensityLayout->addWidget(noiseIntensityValue, Qt::AlignLeft);

    noiseLayout->addWidget(new QLabel("Тип накладываемого шума:"));
    noiseLayout->addLayout(noiseSelectionLayout);
    noiseLayout->addLayout(noiseIntensityLayout);
    noiseLayout->addWidget(m_noiseIntensitySlider);
    rightLayout->addWidget(noiseGroup);

    // Noise reduction
    QGroupBox *noiseReductionGroup = new QGroupBox("Фильтры шумоподавления");
    QVBoxLayout *noiseReductionLayout = new QVBoxLayout(noiseReductionGroup);

    m_noiseReductionBox = new QComboBox();
    m_noiseReductionBox->addItems({"Медианный фильтр", "Фильтр Гаусса"});

    QHBoxLayout *apertureLayout = new QHBoxLayout();
    QLabel *apertureValue = new QLabel("3");

    m_apertureSlider = new QSlider(Qt::Horizontal);
    m_apertureSlider->setRange(3, 15);
    m_apertureSlider->setValue(3);
    m_apertureSlider->setSingleStep(2);
    connect(m_apertureSlider, &QSlider::valueChanged, this, [this, apertureValue](){
        apertureValue->setText(QString("%1").arg(m_apertureSlider->value()));
    });

    apertureLayout->addWidget(new QLabel("Размер апертуры: "));
    apertureLayout->addWidget(apertureValue, Qt::AlignLeft);

    QPushButton *noiseReductionButton = new QPushButton("Применить выбранный фильтр");
    connect(noiseReductionButton, &QPushButton::clicked, this, &MainWindow::applyNoiseFilter);

    noiseReductionLayout->addWidget(new QLabel("Тип фильтра:"));
    noiseReductionLayout->addWidget(m_noiseReductionBox);
    noiseReductionLayout->addLayout(apertureLayout);
    noiseReductionLayout->addWidget(m_apertureSlider);
    noiseReductionLayout->addWidget(noiseReductionButton);
    rightLayout->addWidget(noiseReductionGroup);

    // Sharpening
    QGroupBox *sharpeningGroup = new QGroupBox("Повышение резкости");
    QVBoxLayout *sharpeningLayout = new QVBoxLayout(sharpeningGroup);

    m_sharpeningMethodBox = new QComboBox();
    m_sharpeningMethodBox->addItems({"Простое ядро", "Фильтр Собеля", "Фильтр Превита", "Фильтр Робертса", "Лапласиан"});

    QHBoxLayout *sharpeningValueLayout = new QHBoxLayout();
    QLabel *sharpeningValue = new QLabel("2");

    m_sharpeningSlider = new QSlider(Qt::Horizontal);
    m_sharpeningSlider->setRange(1, 10);
    m_sharpeningSlider->setValue(2);
    connect(m_sharpeningSlider, &QSlider::valueChanged, this, [this, sharpeningValue](){
        sharpeningValue->setText(QString("%1").arg(m_sharpeningSlider->value()));
    });

    sharpeningValueLayout->addWidget(new QLabel("Степень резкости: "));
    sharpeningValueLayout->addWidget(sharpeningValue, Qt::AlignLeft);

    QPushButton *applySharpeningButton = new QPushButton("Применить резкость");
    connect(applySharpeningButton, &QPushButton::clicked, this, &MainWindow::applySharpeningFilter);

    sharpeningLayout->addWidget(new QLabel("Метод:"));
    sharpeningLayout->addWidget(m_sharpeningMethodBox);
    sharpeningLayout->addLayout(sharpeningValueLayout);
    sharpeningLayout->addWidget(m_sharpeningSlider);
    sharpeningLayout->addWidget(applySharpeningButton);

    rightLayout->addWidget(sharpeningGroup);

    // Effect
    QGroupBox *effectGroup = new QGroupBox("Спецэффект");
    QVBoxLayout *effectLayout = new QVBoxLayout(effectGroup);

    QHBoxLayout *effectValueLayout = new QHBoxLayout();

    QLabel *effectRadiusValue = new QLabel("3");

    effectValueLayout->addWidget(new QLabel("Радиус искажения: "));
    effectValueLayout->addWidget(effectRadiusValue, Qt::AlignLeft);

    QSlider *effectSlider = new QSlider(Qt::Horizontal);
    effectSlider->setRange(2, 10);
    effectSlider->setValue(3);
    connect(effectSlider, &QSlider::valueChanged, this, [effectSlider, effectRadiusValue]{
        effectRadiusValue->setText(QString("%1").arg(effectSlider->value()));
    });

    QPushButton *applyEffectButton = new QPushButton("Применить эффект стекла");
    connect(applyEffectButton, &QPushButton::clicked, this, [this, effectSlider](){
        m_processedImage = ImageProcessor::applyGlassEffect(m_processedImage, effectSlider->value());
        updateImage();
    });

    effectLayout->addLayout(effectValueLayout);
    effectLayout->addWidget(effectSlider);
    effectLayout->addWidget(applyEffectButton);

    rightLayout->addWidget(effectGroup);

    rightLayout->addStretch(); // Push widgets to top
    splitter->setStretchFactor(0, 1); // Image takes available space
    splitter->setStretchFactor(1, 0); // Controls take minimum space
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);

    if (m_imageItem && m_imageView) {
        m_imageView->fitInView(m_imageItem, Qt::KeepAspectRatio);
    }
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

void MainWindow::applyNoiseFilter() {
    if (m_processedImage.isNull()) return;

    int kernelSize = m_apertureSlider->value();
    int filter = m_noiseReductionBox->currentIndex();

    if (filter == 0) { // Median filter
        m_processedImage = ImageProcessor::applyMedianFilter(m_processedImage, kernelSize);
    } else { // Gaussian filter
        m_processedImage = ImageProcessor::applyGaussianFilter(m_processedImage, kernelSize);
    }

    updateImage();
}

void MainWindow::applySharpeningFilter() {
    if (m_processedImage.isNull()) return;

    int k = m_sharpeningSlider->value();

    switch (m_sharpeningMethodBox->currentIndex()) {
        case 0: m_processedImage = ImageProcessor::applyBasicSharpeningFilter(m_processedImage, k); break;
        case 1: m_processedImage = ImageProcessor::applySobelFilter(m_processedImage, k);           break;
        case 2: m_processedImage = ImageProcessor::applyPrewittFilter(m_processedImage, k);         break;
        case 3: m_processedImage = ImageProcessor::applyRobertsFilter(m_processedImage, k);         break;
        case 4: m_processedImage = ImageProcessor::applyLaplacian(m_processedImage, k);             break;
        default:
            break;
    }

    updateImage();
}

void MainWindow::updateImage() {
    if (m_processedImage.isNull()) return;

    QPixmap pixmap = QPixmap::fromImage(m_processedImage);

    // If the item doesn't exist yet, create it
    if (!m_imageItem) {
        m_scene->clear();
        m_imageItem = m_scene->addPixmap(pixmap);

        m_imageView->fitInView(m_imageItem, Qt::KeepAspectRatio);
    } else {
        m_imageItem->setPixmap(pixmap);
    }
}

void MainWindow::resetImage() {
    if (m_originalImage.isNull()) return;

    m_processedImage = m_originalImage;
    updateImage();
}
