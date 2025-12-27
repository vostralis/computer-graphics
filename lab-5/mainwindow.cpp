#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_updateTimer = new QTimer(this);
    m_updateTimer->setSingleShot(true);
    m_updateTimer->setInterval(30);
    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::applyTransformations);

    this->setupUI();
    m_imageView->setMouseTracking(true);
    m_imageView->viewport()->installEventFilter(this);

    m_scene->clear();
    m_imageItem = nullptr;
    m_selectionItem = nullptr;
    m_selectionRect = QRect();
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
    m_imageView->setDragMode(QGraphicsView::ScrollHandDrag); // Allow panning
    m_imageView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    splitter->addWidget(m_imageView);

    m_scene = new QGraphicsScene(this);
    m_imageView->setScene(m_scene);

    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    splitter->addWidget(rightPanel);

    // Histogram
    m_chart = new QChart();
    m_chart->setTitle("Гистограмма яркости");
    m_chart->legend()->hide();

    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setFixedHeight(500);
    m_chartView->setFixedWidth(600);
    rightLayout->addWidget(m_chartView);

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

    // Brightness/Contrast
    QGroupBox *bcGroup = new QGroupBox("Яркость и контрастность");
    QFormLayout *bcLayout = new QFormLayout(bcGroup);

    m_brightnessSlider = new QSlider(Qt::Horizontal);
    m_brightnessSlider->setRange(-255, 255);
    m_brightnessLabel = new QLabel("Яркость: 0");

    connect(m_brightnessSlider, &QSlider::valueChanged, this, [this](){
        m_updateTimer->start();
        m_brightnessLabel->setText(QString("Яркость: %1").arg(m_brightnessSlider->value()));
    });

    m_contrastSlider = new QSlider(Qt::Horizontal);
    m_contrastSlider->setRange(0, 10000);
    m_contrastSlider->setValue(5000);
    m_contrastLabel = new QLabel("Контрастность: 0");

    connect(m_contrastSlider, &QSlider::valueChanged, this, [this](){
        m_updateTimer->start();
        QString text = QString("Контрастность: %1").arg(static_cast<int>(m_contrastSlider->value() / 100));
        if (static_cast<int>(m_contrastSlider->value() / 100) < 10) text += "  ";
        else if (text != "100") text += "";
        m_contrastLabel->setText(text);
    });

    bcLayout->addRow(m_brightnessLabel, m_brightnessSlider);
    bcLayout->addRow(m_contrastLabel, m_contrastSlider);
    rightLayout->addWidget(bcGroup);

    QGroupBox *colorGroup = new QGroupBox("Измение цветности");
    QVBoxLayout *colorLayout = new QVBoxLayout(colorGroup);

    m_grayscaleCheckBox = new QCheckBox("Серый");
    connect(m_grayscaleCheckBox, &QCheckBox::toggled, this, &MainWindow::applyTransformations);
    m_negativeCheckBox = new QCheckBox("Негатив");
    connect(m_negativeCheckBox, &QCheckBox::toggled, this, &MainWindow::applyTransformations);

    colorLayout->addWidget(m_grayscaleCheckBox);
    colorLayout->addWidget(m_negativeCheckBox);
    rightLayout->addWidget(colorGroup);

    // Binarization
    QGroupBox *binGroup = new QGroupBox("Бинаризация");
    QVBoxLayout *binLayout = new QVBoxLayout(binGroup);

    m_binariztionMethod = new QComboBox();
    m_binariztionMethod->addItems({"Нет", "Фиксированный порог", "Метод Оцу", "Адаптивный порог"});
    connect(m_binariztionMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyTransformations);

    m_thresholdSlider = new QSlider(Qt::Horizontal);
    m_thresholdSlider->setRange(0, 255);
    m_thresholdSlider->setValue(128);
    QLabel *thresholdLabel = new QLabel("Порог: 128");

    connect(m_thresholdSlider, &QSlider::valueChanged, this, [this, thresholdLabel](){
        m_updateTimer->start();
        thresholdLabel->setText(QString("Порог: %1").arg(m_thresholdSlider->value()));
    });

    binLayout->addWidget(new QLabel("Метод:"));
    binLayout->addWidget(m_binariztionMethod);
    binLayout->addWidget(thresholdLabel);
    binLayout->addWidget(m_thresholdSlider);
    rightLayout->addWidget(binGroup);

    rightLayout->addStretch(); // Push widgets to top
    splitter->setStretchFactor(0, 1); // Image takes available space
    splitter->setStretchFactor(1, 0); // Controls take minimum space

    resize(1200, 800);
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
    m_selectionItem = nullptr;
    m_selectionRect = QRect();
    m_brightnessSlider->setValue(0);
    m_contrastSlider->setValue(5000);
    m_thresholdSlider->setValue(128);
    m_grayscaleCheckBox->setCheckState(Qt::Unchecked);
    m_negativeCheckBox->setCheckState(Qt::Unchecked);
    m_binariztionMethod->setCurrentIndex(0);

    applyTransformations();
}

void MainWindow::updateImage() {
    if (m_processedImage.isNull()) return;

    QPixmap pixmap = QPixmap::fromImage(m_processedImage);

    // If the item doesn't exist yet, create it
    if (!m_imageItem) {
        m_scene->clear();
        m_imageItem = m_scene->addPixmap(pixmap);
        m_imageItem->setZValue(0); // Ensure image is behind the red box
        m_selectionItem = nullptr;

        m_imageView->fitInView(m_imageItem, Qt::KeepAspectRatio);
    } else {
        m_imageItem->setPixmap(pixmap);
    }
}

void MainWindow::resetImage() {
    if (m_originalImage.isNull()) return;

    m_processedImage = m_originalImage;
    m_brightnessSlider->setValue(0);
    m_contrastSlider->setValue(5000);
    m_negativeCheckBox->setChecked(false);
    m_grayscaleCheckBox->setChecked(false);
    m_binariztionMethod->setCurrentIndex(0);

    updateImage();
}

void MainWindow::applyTransformations() {
    if (m_originalImage.isNull()) return;

    QImage base = m_originalImage;
    QImage result = base;

    // Decide ROI
    bool hasROI = !m_selectionRect.isEmpty();
    QRect roi = hasROI ? m_selectionRect : base.rect();

    if (roi.width() <= 0 || roi.height() <= 0) {
        roi = base.rect();
    }

    // Extract region to process
    QImage region = base.copy(roi);

    // Apply transforms
    region = ImageProcessor::applyTransformations(
        region,
        m_brightnessSlider->value(),
        m_contrastSlider->value(),
        m_grayscaleCheckBox->isChecked(),
        m_negativeCheckBox->isChecked()
    );

    // Apply binarization
    int binarizationMode = m_binariztionMethod->currentIndex();
    int threshold = m_thresholdSlider->value();

    if (binarizationMode == 1)
        region = ImageProcessor::binarizeManual(region, threshold);
    else if (binarizationMode == 2)
        region = ImageProcessor::binarizeOtsu(region);
    else if (binarizationMode == 3)
        region = ImageProcessor::binarizeAdaptive(region);

    // Composite result
    QPainter painter(&result);
    painter.drawImage(roi.topLeft(), region);
    painter.end();

    m_processedImage = result;
    updateImage();
    calculateHistogram();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (watched != m_imageView->viewport() || m_originalImage.isNull())
        return QMainWindow::eventFilter(watched, event);

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *e = static_cast<QMouseEvent*>(event);

        if (e->button() == Qt::LeftButton) {
            m_isSelecting = true;
            m_startPoint = m_imageView->mapToScene(e->pos());

            if (!m_selectionItem) {
                m_selectionItem = new QGraphicsRectItem();
                m_selectionItem->setPen(QPen(Qt::red, 2, Qt::DashLine));
                m_selectionItem->setBrush(Qt::NoBrush);
                m_scene->addItem(m_selectionItem);
            }

            m_selectionItem->setRect(QRectF(m_startPoint, m_startPoint));
            m_selectionItem->show();
            return true;
        }

        if (e->button() == Qt::RightButton) {
            if (m_selectionItem) m_selectionItem->hide();
            m_selectionRect = QRect();
            applyTransformations();
            return true;
        }
    }

    if (event->type() == QEvent::MouseMove && m_isSelecting) {
        auto *e = static_cast<QMouseEvent*>(event);
        QPointF current = m_imageView->mapToScene(e->pos());
        m_selectionItem->setRect(QRectF(m_startPoint, current).normalized());
        return true;
    }

    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *e = static_cast<QMouseEvent*>(event);
        if (e->button() == Qt::LeftButton && m_isSelecting) {
            m_isSelecting = false;

            QRectF roughRect = m_selectionItem->rect();
            QRect finalRect = roughRect.toRect();
            m_selectionRect = finalRect.intersected(m_originalImage.rect());

            applyTransformations();
            return true;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::calculateHistogram() {
    QImage source;

    if (!m_selectionRect.isEmpty()) {
        source = m_processedImage.copy(m_selectionRect);
    } else {
        source = m_processedImage;
    }

    if (source.isNull()) return;

    auto histData = ImageProcessor::calculateHistogram(source);

    m_chart->removeAllSeries();
    auto axesX = m_chart->axes(Qt::Horizontal);
    for (auto ax : axesX) m_chart->removeAxis(ax);
    auto axesY = m_chart->axes(Qt::Vertical);
    for (auto ax : axesY) m_chart->removeAxis(ax);

    QBarSet *set = new QBarSet("Luma");
    set->setColor(Qt::black);
    set->setBorderColor(Qt::transparent);

    // Find max for Y-Scaling
    int maxVal = 0;
    for (int val : histData) {
        *set << val;
        if (val > maxVal) maxVal = val;
    }

    QBarSeries *series = new QBarSeries();
    series->append(set);
    series->setBarWidth(1.0); // Make it look solid

    m_chart->addSeries(series);

    // Setup Axes
    QValueAxis *axisX = new QValueAxis();
    axisX->setRange(-5, 260);
    axisX->setLabelFormat("%d");
    axisX->setTickCount(5);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, maxVal);
    axisY->setLabelFormat("%d");

    m_chart->addAxis(axisX, Qt::AlignBottom);
    m_chart->addAxis(axisY, Qt::AlignLeft);

    series->attachAxis(axisX);
    series->attachAxis(axisY);

    m_chart->setTitle(QString("Гистограмма яркости"));
}
