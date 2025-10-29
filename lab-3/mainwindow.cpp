#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QButtonGroup>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Main layout that containes view and control panel layouts
    mainLayout = new QHBoxLayout();

    // Layout that contains object view
    sceneLayout = new QVBoxLayout();

    sceneWidget = new Scene(this);
    Object* sceneObject = sceneWidget->getObject();
    LightSource* sceneLightSource = sceneWidget->getLightSource();
    sceneLayout->addWidget(sceneWidget);
    // ----------------------------------------------------------

    // Layout that contains control panel
    sidebarLayout = new QVBoxLayout();

    // Transformations info
    QGroupBox *transformationsBox = new QGroupBox("Параметры объекта:");
    QVBoxLayout *transformationsBoxLayout = new QVBoxLayout(transformationsBox);

    tLabel = new QLabel("Позиция: (0.00, 0.00, 0.00)");
    rLabel = new QLabel("Поворот: (0.00, 0.00, 0.00)");
    sLabel = new QLabel("Масштаб: 1.50");

    transformationsBoxLayout->addWidget(tLabel);
    transformationsBoxLayout->addWidget(rLabel);
    transformationsBoxLayout->addWidget(sLabel);

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *resetObject = new QPushButton("Сбросить состояние объекта", this);    
    connect(resetObject, &QPushButton::clicked, sceneObject, &Object::reset);

    buttonLayout->addWidget(resetObject);

    // Lighting modes
    QGroupBox *lightingModes = new QGroupBox("");
    QVBoxLayout *lightingModesLayout = new QVBoxLayout(lightingModes);

    simpleModel = new QRadioButton("Простейшая закрасака");
    gouraudShading = new QRadioButton("Закраска Гуро");

    QButtonGroup *viewGroup = new QButtonGroup(this);
    viewGroup->addButton(simpleModel);
    viewGroup->addButton(gouraudShading);
    simpleModel->setChecked(true);
    connect(viewGroup, &QButtonGroup::buttonClicked, this, &MainWindow::onLightingModelChanged);

    onLightingModelChanged(); // Explicit call for the correct button initialization

    lightingModesLayout->addWidget(simpleModel);
    lightingModesLayout->addWidget(gouraudShading);
    // ----------------------------------------------------------

    //
    QGroupBox *somethingBox = new QGroupBox("");
    QVBoxLayout *somethingLayout = new QVBoxLayout(somethingBox);

    resolutionLabel = new QLabel(QString("Разрешение: %1").arg(sceneObject->getResolution()));

    resolutionSlider = new QSlider(Qt::Horizontal);
    resolutionSlider->setRange(3, 180);
    resolutionSlider->setValue(sceneObject->getResolution());
    connect(resolutionSlider, &QSlider::valueChanged, sceneObject, &Object::setResolution);
    connect(resolutionSlider, &QSlider::valueChanged, this, [this](){
        resolutionLabel->setText(QString("Разрешение: %1").arg(resolutionSlider->value()));
    });

    ambientLightIntensityLabel = new QLabel(QString("Интенсивность фонового света: %1").arg(sceneWidget->getAmbientLightIntensity()));
    diffuseReflectionCoefficientLabel = new QLabel(QString("Коэффициент диффузного отражения: %1").arg(sceneObject->getDiffuseCoefficient()));
    ambientReflectionCoefficientLabel = new QLabel(QString("Коэффициент зеркального отражения: %1").arg(sceneObject->getAmbientCoefficient()));
    lightSourceIntensityLabel = new QLabel(QString("Интенсивность источника света: %1").arg(sceneLightSource->intensity()));


    ambientLightIntensitySlider = new QSlider(Qt::Horizontal);
    ambientLightIntensitySlider->setRange(0, 100);
    ambientLightIntensitySlider->setValue(static_cast<int>(sceneWidget->getAmbientLightIntensity() * 100));
    connect(ambientLightIntensitySlider, &QSlider::valueChanged, sceneWidget, &Scene::setAmbientLightIntensity);
    connect(ambientLightIntensitySlider, &QSlider::valueChanged, this, [this](){
        ambientLightIntensityLabel->setText(QString("Интенсивность фонового света: %1").arg(sceneWidget->getAmbientLightIntensity()));
    });

    diffuseReflectionCoefficientSlider = new QSlider(Qt::Horizontal);
    diffuseReflectionCoefficientSlider->setRange(0, 100);
    diffuseReflectionCoefficientSlider->setValue(static_cast<int>(sceneObject->getDiffuseCoefficient() * 100));
    connect(diffuseReflectionCoefficientSlider, &QSlider::valueChanged, sceneObject, &Object::setDiffuseReflectionCoefficient);
    connect(diffuseReflectionCoefficientSlider, &QSlider::valueChanged, this, [this, sceneObject](){
        diffuseReflectionCoefficientLabel->setText(QString("Коэффициент диффузного отражения: %1").arg(sceneObject->getDiffuseCoefficient()));
    });

    ambientReflectionCoefficientSlider = new QSlider(Qt::Horizontal);
    ambientReflectionCoefficientSlider->setRange(0, 100);
    ambientReflectionCoefficientSlider->setValue(static_cast<int>(sceneObject->getAmbientCoefficient() * 100));
    connect(ambientReflectionCoefficientSlider, &QSlider::valueChanged, sceneObject, &Object::setAmbientReflectionCoefficient);
    connect(ambientReflectionCoefficientSlider, &QSlider::valueChanged, this, [this, sceneObject](){
        ambientReflectionCoefficientLabel->setText(QString("Коэффициент зеркального отражения: %1").arg(sceneObject->getAmbientCoefficient()));
    });

    lightSourceIntensitySlider = new QSlider(Qt::Horizontal);
    lightSourceIntensitySlider->setRange(0, 100);
    lightSourceIntensitySlider->setValue(static_cast<int>(sceneLightSource->intensity() * 100));
    connect(lightSourceIntensitySlider, &QSlider::valueChanged, sceneLightSource, &LightSource::setIntensity);
    connect(lightSourceIntensitySlider, &QSlider::valueChanged, this, [this, sceneLightSource](){
        lightSourceIntensityLabel->setText(QString("Интенсивность источника света: %1").arg(sceneLightSource->intensity()));
    });

    somethingLayout->addWidget(resolutionLabel);
    somethingLayout->addWidget(resolutionSlider);
    somethingLayout->addWidget(ambientLightIntensityLabel);
    somethingLayout->addWidget(ambientLightIntensitySlider);
    somethingLayout->addWidget(diffuseReflectionCoefficientLabel);
    somethingLayout->addWidget(diffuseReflectionCoefficientSlider);
    somethingLayout->addWidget(ambientReflectionCoefficientLabel);
    somethingLayout->addWidget(ambientReflectionCoefficientSlider);
    somethingLayout->addWidget(lightSourceIntensityLabel);
    somethingLayout->addWidget(lightSourceIntensitySlider);
    // ----------------------------------------------------------


    sidebarLayout->addWidget(transformationsBox);
    sidebarLayout->addLayout(buttonLayout);
    sidebarLayout->addWidget(lightingModes);
    sidebarLayout->addWidget(somethingBox);

    sidebarLayout->addStretch(1);

    connect(sceneObject, &Object::stateChanged, this, &MainWindow::updateLabels);
    // ----------------------------------------------------------

    mainLayout->addLayout(sceneLayout, 7);
    mainLayout->addLayout(sidebarLayout, 3);

    // Main widget
    mainContainer = new QWidget(this);
    mainContainer->setLayout(mainLayout);
    setCentralWidget(mainContainer);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateLabels(QVector3D position, float rotationX, float rotationY, float rotationZ, float scale) {
    QString positionText = QString("Позиция: (%1, %2, %3)")
                               .arg(position.x(), 0, 'f', 2)
                               .arg(position.y(), 0, 'f', 2)
                               .arg(position.z(), 0, 'f', 2);

    tLabel->setText(positionText);

    QString rotationText = QString("Поворот: (%1, %2, %3)")
                               .arg(rotationX, 0, 'f', 2)
                               .arg(rotationY, 0, 'f', 2)
                               .arg(rotationZ, 0, 'f', 2);

    rLabel->setText(rotationText);

    sLabel->setText(QString("Масштаб: %1").arg(scale, 0, 'f', 2));
}

void MainWindow::onLightingModelChanged() {
    if (simpleModel->isChecked()) {
        sceneWidget->setLightingModel(Scene::LightingMode::Simple);
    } else {
        sceneWidget->setLightingModel(Scene::LightingMode::GouraudShading);
    }
}
