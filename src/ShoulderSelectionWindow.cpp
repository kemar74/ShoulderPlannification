#include "ShoulderSelectionWindow.h"
#include "ui_ShoulderSelectionWindow.h"

#include <QLayout>
#include <QPushButton>

ShoulderSelectionWindow::ShoulderSelectionWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ShoulderSelectionWindow)
{
    ui->setupUi(this);

    this->viewer = new ShoulderSelectionViewer(ShoulderSelectionViewer::NORMAL, this);
    ui->gridLayout->addWidget(viewer, 0, 2);

    this->ui->init_plannif_button->hide();

    this->viewer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->viewer->hide();

    connect(ui->selectGlene, &QRadioButton::toggled, viewer, [&](bool display) {
        if (display)
            viewer->setSelectionMode(ShoulderSelectionViewer::PointSelectionState::GLENE);
    });
    connect(ui->selectTrigone, &QRadioButton::toggled, viewer, [&](bool display) {
        if (display)
            viewer->setSelectionMode(ShoulderSelectionViewer::PointSelectionState::TRIGONE);
    });
    connect(ui->selectScapula, &QRadioButton::toggled, viewer, [&](bool display) {
        if (display)
            viewer->setSelectionMode(ShoulderSelectionViewer::PointSelectionState::SCAPULA);
    });
    connect(ui->selectNone, &QRadioButton::toggled, viewer, [&](bool display) {
        if (display)
            viewer->setSelectionMode(ShoulderSelectionViewer::PointSelectionState::NONE);
    });

    connect(ui->displaySagital, &QCheckBox::toggled, viewer, &ShoulderSelectionViewer::setSagitalPlaneDisplay);
    connect(ui->displayAxial, &QCheckBox::toggled, viewer, &ShoulderSelectionViewer::setAxialPlaneDisplay);
    connect(ui->displayCoronal, &QCheckBox::toggled, viewer, &ShoulderSelectionViewer::setCoronalPlaneDisplay);
    connect(ui->displayFriedman, &QCheckBox::toggled, viewer, &ShoulderSelectionViewer::setFriedmanAxisDisplay);

    connect(ui->init_plannif_button, &QPushButton::pressed, viewer, &ShoulderSelectionViewer::initPlannificationInput);
    connect(ui->init_plannif_button, &QPushButton::pressed, this, [&]() {
        ui->selectNone->setChecked(true);
    });
    connect(ui->inclinaison_input, SIGNAL(valueChanged(double)), viewer, SLOT(setInclinaison(double)));
    connect(ui->version_input, SIGNAL(valueChanged(double)), viewer, SLOT(setVersion(double)));
    connect(ui->rotation_input, SIGNAL(valueChanged(double)), viewer, SLOT(setRotation(double)));
    connect(ui->actionCharger, &QAction::triggered, this, &ShoulderSelectionWindow::openStlFile);
    connect(ui->first_model_loading_button, &QPushButton::pressed, this, &ShoulderSelectionWindow::openStlFile);
    connect(ui->actionExporter, &QAction::triggered, this, &ShoulderSelectionWindow::exportResultingMatrix);
    connect(ui->export_button, &QPushButton::clicked, this, &ShoulderSelectionWindow::exportResultingMatrix);
    connect(ui->display_wireframe, &QCheckBox::toggled, this, [&](bool wireframe) { this->viewer->useWireframeMode = wireframe; viewer->update(); });
    connect(ui->actionAide, &QAction::triggered, viewer, &ShoulderSelectionViewer::help);

    connect(ui->planification_axis, &QGroupBox::toggled, this, &ShoulderSelectionWindow::setToPlanificationAxisMode);
    connect(ui->points_definition, &QGroupBox::toggled, this, &ShoulderSelectionWindow::setToPointsDefinitionMode);


    ui->displaySagital->setChecked(false);
    ui->displayAxial->setChecked(true);
    ui->displayCoronal->setChecked(true);
    ui->displayFriedman->setChecked(true);
}

ShoulderSelectionWindow::~ShoulderSelectionWindow()
{
    delete ui;
    delete viewer;
}

void ShoulderSelectionWindow::openStlFile()
{
    QString q_filename = QFileDialog::getOpenFileName(this, QString("Ouvrir un fichier STL"), QString(), QString("Fichier (*.stl)"));
    if (q_filename.isEmpty()) return; // Cancel the action if no file has been selected
    std::string filename = q_filename.toStdString();
    this->viewer->show();
    this->viewer->openStlFile(filename);
    this->ui->selection_screen->hide();
    this->ui->options_frame->setEnabled(true);
}

void ShoulderSelectionWindow::exportResultingMatrix()
{
    Matrix mat = this->viewer->computePlannificationTransformMatrix();
    QMessageBox msgBox;
    msgBox.setText(QString::fromStdString("Matrice de transformation : \n" +
                                          mat.transpose().toString() +
                                          "\n\nPoint d'entrée :\n"
                                          + std::to_string(mat[0][3]) + " " + std::to_string(mat[1][3]) + " " + std::to_string(mat[2][3])
                                          +
                    "\n\nMatrice en une ligne [column-major] : \n" +
                    mat.displayValuesOneLine() +
                    "\n\nMatrice en une ligne [row-major] : \n" +
                    mat.transpose().displayValuesOneLine()
                    ));
    msgBox.setTextInteractionFlags(Qt::TextSelectableByMouse);
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
    int ret = msgBox.exec();

    QString fileName;
    switch (ret) {
        case QMessageBox::Save:
            fileName = QFileDialog::getSaveFileName(this, "Enregistrer la matrice");
            if (fileName.isEmpty())
                return;
            else {
                QFile file(fileName);
                if (!file.open(QIODevice::WriteOnly)) {
                    QMessageBox::information(this, tr("Unable to open file"),
                        file.errorString());
                    return;
                }
                QTextStream out(&file);
//                qDebug() << QString::fromStdString(mat.displayValues());
                out << QString::fromStdString(mat.transpose().displayValues());
            }
            break;
        case QMessageBox::Cancel:
            // Cancel was clicked
            break;
        default:
            // should never be reached
            break;
    }
}

void ShoulderSelectionWindow::setToPlanificationAxisMode(bool toggled)
{
    if (toggled) {
        this->ui->points_definition->setChecked(false);
        this->ui->selectNone->setChecked(true);
        this->viewer->initPlannificationInput();
    }
}

void ShoulderSelectionWindow::setToPointsDefinitionMode(bool toggled)
{
    if (toggled) {
        this->ui->planification_axis->setChecked(false);
        this->viewer->backToPointSelection();
    }
}
