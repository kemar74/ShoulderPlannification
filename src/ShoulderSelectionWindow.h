#ifndef SHOULDERSELECTIONWINDOW_H
#define SHOULDERSELECTIONWINDOW_H

//#include <GL/glew.h>
#include <QMainWindow>
#include "ShoulderSelectionViewer.h"


QT_BEGIN_NAMESPACE
namespace Ui { class ShoulderSelectionWindow; }
QT_END_NAMESPACE

class ShoulderSelectionWindow : public QMainWindow
{
    Q_OBJECT

public:
    ShoulderSelectionWindow(QWidget *parent = nullptr);
    ~ShoulderSelectionWindow();

protected Q_SLOTS:
    void openStlFile();
    void exportResultingMatrix();

    void setToPlanificationAxisMode(bool toggled);
    void setToPointsDefinitionMode(bool toggled);

private:
    Ui::ShoulderSelectionWindow *ui;

    ShoulderSelectionViewer* viewer;
};
#endif // SHOULDERSELECTIONWINDOW_H
