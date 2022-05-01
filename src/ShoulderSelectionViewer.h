#ifndef SHOULDERSELECTIONVIEWER_H
#define SHOULDERSELECTIONVIEWER_H

//#include <GL/glew.h>
#include "Interface/ControlPoint.h"
#include "Interface/Slider3D.h"
#include <QGLViewer/qglviewer.h>


#define PI 3.141592653589793238462643383279502884

class ShoulderSelectionViewer : public QGLViewer
{
    Q_OBJECT
public:
    enum VIEWPORT_TYPE {
        FRONT  = 0,
        SIDE   = 1,
        TOP    = 2,
        NORMAL = 3
    };


    ShoulderSelectionViewer(VIEWPORT_TYPE type = NORMAL, QWidget* parent = nullptr);
    ~ShoulderSelectionViewer() {}

    void init();
    void draw();

    void mousePressEvent(QMouseEvent* event);

    Mesh shoulderMesh;
    Mesh sagitalPlaneMesh;
    Mesh axialPlaneMesh;
    Mesh coronalPlaneMesh;
    Mesh friedmanAxisMesh;

    std::unique_ptr<ControlPoint> centre_glene;
    std::unique_ptr<ControlPoint> trigone;
    std::unique_ptr<ControlPoint> scapula;

    std::unique_ptr<ControlPoint> entryPointOnFriedman;

    bool centre_glene_validated = false;
    bool trigone_validated = false;
    bool scapula_validated = false;

    enum PointSelectionState {
        NONE, GLENE, TRIGONE, SCAPULA
    };
    PointSelectionState currentSelection = NONE;

    bool displayingSagitalPlane;
    bool displayingAxialPlane;
    bool displayingCoronalPlane;
    bool displayingFriedmanAxis;

    void computeSagitalPlaneMesh();
    void computeAxialPlaneMesh();
    void computeCoronalPlaneMesh();
    void computeFriedmanAxisMesh();

    Vector3 getGlenePosition() { return this->centre_glene->getPosition(); }
    Vector3 getTrigonePosition() { return this->trigone->getPosition(); }
    Vector3 getScapulaPosition() { return this->scapula->getPosition(); }
    Vector3 getGleneToTrigoneVector();
    Vector3 getScapulaProjectionOnFriedman();
    Vector3 getScapulaToFriedmanVector();
    Vector3 getScapulaToFriedmanBivector();

    float getDistanceToEntryPoint() { return getGleneToTrigoneVector().norm() / 5.f; }
    // Values computed at the end, which will be converted into a transform matrix and exported
    Vector3 plannifDirVector;
    Vector3 plannifNormalVector;
    Vector3 plannifBinormalVector;

    Matrix computePlannificationTransformMatrix();

    Mesh dirMesh, normalMesh, binormalMesh;

    float inclinaison;
    float version;

    // Should not be in the class, but meh!
    float degToRad(float degrees);
    float radToDeg(float rad);

    bool useWireframeMode = false;

    QString helpString() const;


public Q_SLOTS:
    PointSelectionState setSelectionMode(PointSelectionState newState);
    void computeAllPlanes();

    bool setSagitalPlaneDisplay(bool display);
    bool setAxialPlaneDisplay(bool display);
    bool setCoronalPlaneDisplay(bool display);
    bool setFriedmanAxisDisplay(bool display);

    void backToPointSelection();
    void initPlannificationInput();
    void setInclinaison(double newInclinaison);
    void setVersion(double newVersion);

    void moveEntryPointToFitShoulder();
    void recomputeEntryPointPosition();

    void openStlFile(std::string filename);
};

#endif // SHOULDERSELECTIONVIEWER_H
