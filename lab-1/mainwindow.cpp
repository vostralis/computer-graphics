#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>

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
    sceneLayout->addWidget(sceneWidget);
    // ----------------------------------------------------------

    // Layout that contains control panel
    sidebarLayout = new QVBoxLayout();

    QGroupBox* transformationsBox = new QGroupBox("Параметры объекта:");
    QVBoxLayout* transformationsBoxLayout = new QVBoxLayout(transformationsBox);

    tLabel = new QLabel("Позиция: (0, 0, 0)");
    rLabel = new QLabel("Поворот: (0, 0, 0)");
    sLabel = new QLabel("Масштаб: 1.0");

    transformationsBoxLayout->addWidget(tLabel);
    transformationsBoxLayout->addWidget(rLabel);
    transformationsBoxLayout->addWidget(sLabel);

    QSpinBox *transitionDurationSpinbox = new QSpinBox(this);
    transitionDurationSpinbox->setRange(30, 240);
    transitionDurationSpinbox->setValue(60);
    QSpinBox *randomDurationSpinBox = new QSpinBox(this);
    randomDurationSpinBox->setRange(60, 1200);
    randomDurationSpinBox->setValue(180);
    QDoubleSpinBox *maxSpeedSpinBox = new QDoubleSpinBox(this);
    maxSpeedSpinBox->setRange(0.01, 0.1);
    maxSpeedSpinBox->setSingleStep(0.01);
    maxSpeedSpinBox->setValue(0.05);

    connect(transitionDurationSpinbox, QOverload<int>::of(&QSpinBox::valueChanged), sceneObject, &Object::setTransitionDuration);
    connect(randomDurationSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), sceneObject, &Object::setRandomChangeDuration);
    connect(maxSpeedSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), sceneObject, &Object::setMaxRotationSpeed);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *toggleAnimationButton = new QPushButton("Старт/стоп анимации", this);
    QPushButton *resetObject = new QPushButton("Сбросить состояние объекта", this);

    connect(toggleAnimationButton, &QPushButton::clicked, sceneWidget, &Scene::toggleAnimation);
    connect(resetObject, &QPushButton::clicked, sceneObject, &Object::reset);

    buttonLayout->addWidget(toggleAnimationButton);
    buttonLayout->addWidget(resetObject);

    sidebarLayout->addWidget(transformationsBox);
    sidebarLayout->addWidget(new QLabel("Параметры анимации"));
    sidebarLayout->addWidget(new QLabel("Длительность перехода:"));
    sidebarLayout->addWidget(transitionDurationSpinbox);
    sidebarLayout->addWidget(new QLabel("Длительность смены случайного направления:"));
    sidebarLayout->addWidget(randomDurationSpinBox);
    sidebarLayout->addWidget(new QLabel("Максимальная угловая скорость вращения (рад/с):"));
    sidebarLayout->addWidget(maxSpeedSpinBox);
    sidebarLayout->addLayout(buttonLayout);

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

