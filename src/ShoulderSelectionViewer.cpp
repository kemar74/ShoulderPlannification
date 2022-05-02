#include "ShoulderSelectionViewer.h"
#include "Utils/stl_reader.h"
#include "DataStructure/Vector3.h"
#include "Utils/Collisions.h"
#include <QMouseEvent>
#include "Graphics/Mesh.h"
#include <QGLViewer/manipulatedCameraFrame.h>

ShoulderSelectionViewer::ShoulderSelectionViewer(VIEWPORT_TYPE type, QWidget* parent)
    : QGLViewer(parent)
{
    if (type != VIEWPORT_TYPE::NORMAL) {
      // Move camera according to viewer type (on X, Y or Z axis)
      camera()->setPosition(Vector3(
                                (type == SIDE) ? 1.0 : 0.0,
                                (type == FRONT) ? 1.0 : 0.0,
                                (type == TOP) ? 1.0 : 0.0));
      camera()->lookAt(sceneCenter());

      camera()->setType(qglviewer::Camera::ORTHOGRAPHIC);
      camera()->showEntireScene();

      // Forbid rotation
      qglviewer::WorldConstraint *constraint = new qglviewer::WorldConstraint();
      constraint->setRotationConstraintType(qglviewer::AxisPlaneConstraint::FORBIDDEN);
      camera()->frame()->setConstraint(constraint);
    }
}
void ShoulderSelectionViewer::init()
{
    glDisable(GL_CULL_FACE);
    restoreStateFromFile();
    setSceneRadius(500.0);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    GlobalsGL::generateBuffers();

    this->camera()->setType(qglviewer::Camera::ORTHOGRAPHIC);

    setTextIsEnabled(true);
    setMouseTracking(true);

    const char* vNoShader = ":/src/Shaders/no_vertex_shader.glsl";
    const char* fNoShader = ":/src/Shaders/no_fragment_shader.glsl";
    const char* vShader_grid = ":/src/Shaders/vertex_shader_gouraud.glsl";
    const char* fShader_grid = ":/src/Shaders/fragment_shader_gouraud.glsl";

    glEnable              ( GL_DEBUG_OUTPUT );
    GlobalsGL::f()->glDebugMessageCallback( GlobalsGL::MessageCallback, 0 );

    Shader::default_shader = std::make_shared<Shader>(vNoShader, fNoShader);
    ControlPoint::base_shader = std::make_shared<Shader>(vNoShader, fNoShader);
    ControlPoint::base_shader->setVector("color", std::vector<float>({160/255.f, 5/255.f, 0/255.f, 1.f}));
    Shader::default_shader->setVector("color", std::vector<float>({160/255.f, 5/255.f, 0/255.f, 1.f}));

    shoulderMesh.shader = std::make_shared<Shader>(vShader_grid, fShader_grid);

    sagitalPlaneMesh.shader = std::make_shared<Shader>(vNoShader, fNoShader);
    sagitalPlaneMesh.shader->setVector("color", std::vector<float>({.1f, .5f, 1.f, .5f}));
    axialPlaneMesh.shader = std::make_shared<Shader>(vNoShader, fNoShader);
    axialPlaneMesh.shader->setVector("color", std::vector<float>({.5f, 1.f, .1f, .5f}));
    coronalPlaneMesh.shader = std::make_shared<Shader>(vNoShader, fNoShader);
    coronalPlaneMesh.shader->setVector("color", std::vector<float>({1.f, .1f, .5f, .5f}));
    friedmanAxisMesh.shader = std::make_shared<Shader>(vNoShader, fNoShader);
    friedmanAxisMesh.shader->setVector("color", std::vector<float>({1.f, .0f, .0f, .8f}));

    centre_glene = std::make_unique<ControlPoint>(Vector3(), 5.f);
    centre_glene->setGrabberStateColor(GrabberState::ACTIVE  , std::vector<float>({1.f, .3f, .1f, 1.f}));
    centre_glene->setGrabberStateColor(GrabberState::INACTIVE, std::vector<float>({6.f, .1f, .0f, 1.f}));
    centre_glene->allowAllAxisTranslation(true);
    centre_glene->hide();

    trigone = std::make_unique<ControlPoint>(Vector3(), 5.f);
    trigone->setGrabberStateColor(GrabberState::ACTIVE  , std::vector<float>({.3f, 1.f, .1f, 1.f}));
    trigone->setGrabberStateColor(GrabberState::INACTIVE, std::vector<float>({.1f, .6f, .0f, 1.f}));
    trigone->allowAllAxisTranslation(true);
    trigone->hide();

    scapula = std::make_unique<ControlPoint>(Vector3(), 5.f);
    scapula->setGrabberStateColor(GrabberState::ACTIVE  , std::vector<float>({.1f, .3f, 1.f, 1.f}));
    scapula->setGrabberStateColor(GrabberState::INACTIVE, std::vector<float>({.0f, .1f, .6f, 1.f}));
    scapula->allowAllAxisTranslation(true);
    scapula->hide();

//    this->openStlFile("C:/codes/Qt/ShoulerPlannification/src/asset/Scapula.stl");

    this->entryPointOnFriedman = std::make_unique<ControlPoint>();
    this->entryPointOnFriedman->hide();

    dirMesh = Mesh(std::make_shared<Shader>(*Shader::default_shader));
    normalMesh = Mesh(std::make_shared<Shader>(*Shader::default_shader));
    binormalMesh = Mesh(std::make_shared<Shader>(*Shader::default_shader));

    Mesh::setShaderToAllMeshesWithoutShader(*Shader::default_shader);

    QObject::connect(centre_glene.get(), &ControlPoint::modified, this, &ShoulderSelectionViewer::computeAllPlanes);
    QObject::connect(trigone.get(), &ControlPoint::modified, this, &ShoulderSelectionViewer::computeAllPlanes);
    QObject::connect(scapula.get(), &ControlPoint::modified, this, &ShoulderSelectionViewer::computeAllPlanes);

    QObject::connect(entryPointOnFriedman.get(), &ControlPoint::modified, this, &ShoulderSelectionViewer::moveEntryPointToFitShoulder);

//    startAnimation();
    this->computeAllPlanes();
    QGLViewer::init();
}

void ShoulderSelectionViewer::draw()
{
    glClear(GL_DEPTH_BUFFER_BIT);

    float pMatrix[16];
    float mvMatrix[16];
    camera()->getProjectionMatrix(pMatrix);
    camera()->getModelViewMatrix(mvMatrix);

    Shader::applyToAllShaders([&](std::shared_ptr<Shader> shader) -> void {
        shader->setMatrix("proj_matrix", pMatrix);
        shader->setMatrix("norm_matrix", Matrix(4, 4, mvMatrix).transpose().inverse());
        shader->setMatrix("mv_matrix", mvMatrix);
    });

    if (useWireframeMode)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    this->shoulderMesh.display();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    this->centre_glene->display();
    this->trigone->display();
    this->scapula->display();

    this->sagitalPlaneMesh.display();
    this->axialPlaneMesh.display();
    this->coronalPlaneMesh.display();
    this->friedmanAxisMesh.display(GL_LINES, 3.f);

    this->entryPointOnFriedman->display();


    dirMesh.shader->setVector("color", std::vector<float>({.0f, .0f, 1.f, 1.f}));
    normalMesh.shader->setVector("color", std::vector<float>({.0f, 1.f, .0f, 1.f}));
    binormalMesh.shader->setVector("color", std::vector<float>({1.f, .0f, .0f, 1.f}));
    this->dirMesh.display(GL_LINES, 3.f);
    this->normalMesh.display(GL_LINES, 3.f);
    this->binormalMesh.display(GL_LINES, 3.f);
    QGLViewer::draw();

}

void ShoulderSelectionViewer::mousePressEvent(QMouseEvent *event)
{
    qglviewer::Vec _orig, _dir;
    this->camera()->convertClickToLine(event->pos(), _orig, _dir);
    Vector3 orig(_orig), dir(_dir);

    if (currentSelection != NONE) {
        Vector3 collisionPoint(false);
        std::vector<std::vector<Vector3>> triangles = this->shoulderMesh.getTriangles();
        for (auto& t : triangles) {
            Vector3 v1 = t[0];
            Vector3 v2 = t[1];
            Vector3 v3 = t[2];
            Vector3 collision = Collision::segmentToTriangleCollision(orig, orig + dir * 1000.f, v1, v2, v3);
            if (collision.isValid()) {
                if (!collisionPoint.isValid() || (collision - orig) < (collisionPoint - orig))
                    collisionPoint = collision;
            }
        }
        if (collisionPoint.isValid()) {
            if (currentSelection == GLENE) {
                this->centre_glene->move(collisionPoint);
                this->centre_glene->show();
                this->centre_glene_validated = true;
            } else if (currentSelection == TRIGONE) {
                this->trigone->move(collisionPoint);
                this->trigone->show();
                this->trigone_validated = true;
            } else if (currentSelection == SCAPULA) {
                this->scapula->move(collisionPoint);
                this->scapula->show();
                this->scapula_validated = true;
            }
            this->computeAllPlanes();
        }
    }

    QGLViewer::mousePressEvent(event);
}

void ShoulderSelectionViewer::computeSagitalPlaneMesh()
{
    if (this->centre_glene_validated && this->trigone_validated && this->scapula_validated)
    {
        Vector3 g = getGlenePosition();
        Vector3 scapulaToFriedman = getScapulaToFriedmanVector();
        Vector3 bivector = getScapulaToFriedmanBivector().normalize() * scapulaToFriedman.norm();

        this->sagitalPlaneMesh.fromArray(std::vector<Vector3>({
                                                                  (g + scapulaToFriedman * 2.f) + bivector * 2.f,
                                                                  (g - scapulaToFriedman * 2.f) + bivector * 2.f,
                                                                  (g - scapulaToFriedman * 2.f) - bivector * 2.f,

                                                                  (g + scapulaToFriedman * 2.f) + bivector * 2.f,
                                                                  (g - scapulaToFriedman * 2.f) - bivector * 2.f,
                                                                  (g + scapulaToFriedman * 2.f) - bivector * 2.f,

                                                              }));
    }
}

void ShoulderSelectionViewer::computeAxialPlaneMesh()
{
    if (this->centre_glene_validated && this->trigone_validated && this->scapula_validated)
    {
        Vector3 g = getGlenePosition();
        Vector3 t = getTrigonePosition();

        Vector3 gleneToTrigone = getGleneToTrigoneVector();
        Vector3 bivector = getScapulaToFriedmanBivector().normalize() * getScapulaToFriedmanVector().norm();

        this->axialPlaneMesh.fromArray(std::vector<Vector3>({
                                                                  (g - gleneToTrigone) + bivector * 2,
                                                                  (g - gleneToTrigone) - bivector * 2,
                                                                  (t + gleneToTrigone) - bivector * 2,

                                                                  (g - gleneToTrigone) + bivector * 2,
                                                                  (t + gleneToTrigone) + bivector * 2,
                                                                  (t + gleneToTrigone) - bivector * 2,
                                                              }));
    }
}

void ShoulderSelectionViewer::computeCoronalPlaneMesh()
{
    if (this->centre_glene_validated && this->trigone_validated && this->scapula_validated)
    {
        Vector3 g = getGlenePosition();
        Vector3 t = getTrigonePosition();

        Vector3 gleneToTrigone = getGleneToTrigoneVector();
        Vector3 scapulaToFriedman = getScapulaToFriedmanVector();

        this->coronalPlaneMesh.fromArray(std::vector<Vector3>({
                                                                  (g - gleneToTrigone) + scapulaToFriedman * 2,
                                                                  (g - gleneToTrigone) - scapulaToFriedman * 2,
                                                                  (t + gleneToTrigone) - scapulaToFriedman * 2,

                                                                  (g - gleneToTrigone) + scapulaToFriedman * 2,
                                                                  (t + gleneToTrigone) + scapulaToFriedman * 2,
                                                                  (t + gleneToTrigone) - scapulaToFriedman * 2,
                                                              }));
    }
}

void ShoulderSelectionViewer::computeFriedmanAxisMesh()
{
    if (this->centre_glene_validated && this->trigone_validated)
    {
        this->friedmanAxisMesh.fromArray(std::vector<Vector3>({
                                                                  getGlenePosition() - getGleneToTrigoneVector() * 5.f,
                                                                  getTrigonePosition() + getGleneToTrigoneVector() * 5.f,
                                                              }));
    }
}

Vector3 ShoulderSelectionViewer::getGleneToTrigoneVector()
{
    return getTrigonePosition() - getGlenePosition();
}

Vector3 ShoulderSelectionViewer::getScapulaProjectionOnFriedman()
{
    Vector3 gleneToTrigone = getGleneToTrigoneVector();
    Vector3 gleneToScapula = getScapulaPosition() - getGlenePosition();
    return getGlenePosition() + gleneToTrigone * (gleneToScapula.dot(gleneToTrigone) / (gleneToTrigone).dot(gleneToTrigone));
}

Vector3 ShoulderSelectionViewer::getScapulaToFriedmanVector()
{
    return getScapulaProjectionOnFriedman() - getScapulaPosition();
}

Vector3 ShoulderSelectionViewer::getScapulaToFriedmanBivector()
{
    return getScapulaToFriedmanVector().cross(getGleneToTrigoneVector());
}

Matrix ShoulderSelectionViewer::computePlannificationTransformMatrix()
{
    Vector3 entryPoint = this->entryPointOnFriedman->getPosition();
    Vector3 fromGleneToEntry = entryPoint - getGlenePosition();
    Vector3 vectorFromCentreGlene = getGleneToTrigoneVector().normalize();
//    if ((entryPoint - getGlenePosition()).norm2() > 1e-8)
//        vectorFromCentreGlene *= -(entryPoint - getGlenePosition()).norm();
//        vectorFromCentreGlene *= -getDistanceToEntryPoint();
//    vectorFromCentreGlene += fromGleneToEntry;
    float versionRad = -degToRad(this->version);
    float inclinaisonRad = -degToRad(this->inclinaison);

    this->plannifDirVector = getGleneToTrigoneVector().normalized();
    this->plannifNormalVector = getScapulaToFriedmanVector().normalized();
    this->plannifBinormalVector = getScapulaToFriedmanBivector().normalized();

    Vector3 versionRotationAxis = getScapulaToFriedmanVector();
    vectorFromCentreGlene.rotate(versionRad, versionRotationAxis);
    plannifDirVector.rotate(versionRad, versionRotationAxis);
    plannifNormalVector.rotate(versionRad, versionRotationAxis);
    plannifBinormalVector.rotate(versionRad, versionRotationAxis);

    Vector3 inclinaisonRotationAxis = vectorFromCentreGlene.cross(getScapulaToFriedmanVector());
    vectorFromCentreGlene.rotate(inclinaisonRad, inclinaisonRotationAxis);
    plannifDirVector.rotate(inclinaisonRad, inclinaisonRotationAxis);
    plannifNormalVector.rotate(inclinaisonRad, inclinaisonRotationAxis);
    plannifBinormalVector.rotate(inclinaisonRad, inclinaisonRotationAxis);

    // Last operation to get the same orientation as the slide 19:
    plannifDirVector.rotate(-PI/2.f, plannifBinormalVector);
    plannifNormalVector.rotate(-PI/2.f, plannifBinormalVector);
    Vector3 displayedEntryPoint = getGlenePosition() + vectorFromCentreGlene.normalized() * -getDistanceToEntryPoint() + fromGleneToEntry;

    dirMesh.fromArray({displayedEntryPoint, displayedEntryPoint + plannifDirVector * 100.f});
    normalMesh.fromArray({displayedEntryPoint, displayedEntryPoint + plannifNormalVector * 100.f});
    binormalMesh.fromArray({displayedEntryPoint, displayedEntryPoint + plannifBinormalVector * 100.f});

    std::vector<float> matrix_data = {
                            plannifDirVector.x     , plannifDirVector.y     , plannifDirVector.z     , 0,
                            plannifNormalVector.x  , plannifNormalVector.y  , plannifNormalVector.z  , 0,
                            plannifBinormalVector.x, plannifBinormalVector.y, plannifBinormalVector.z, 0,
                            entryPoint.x           , entryPoint.y           , entryPoint.z           , 1
                         };
    Matrix transform(4, 4, matrix_data.data());
    std::cout << transform << std::endl;
    return transform;
}

float ShoulderSelectionViewer::degToRad(float degrees)
{
    return degrees * (PI / 180.f);
}

float ShoulderSelectionViewer::radToDeg(float rad)
{
    return rad * (180.f / PI);
}

ShoulderSelectionViewer::PointSelectionState ShoulderSelectionViewer::setSelectionMode(PointSelectionState newState)
{
    if (newState == PointSelectionState::GLENE) {
//        this->centre_glene->show();
    } else if (newState == PointSelectionState::TRIGONE) {
//        this->trigone->show();
    } else if (newState == PointSelectionState::SCAPULA) {
//        this->scapula->show();
    } else if (newState == PointSelectionState::NONE) {
    }
    this->currentSelection = newState;
    this->update();
    return newState;
}

bool ShoulderSelectionViewer::setSagitalPlaneDisplay(bool display)
{
    this->displayingSagitalPlane = display;
    if (display)
        sagitalPlaneMesh.show();
    else
        sagitalPlaneMesh.hide();
    this->update();
    return display;
}

bool ShoulderSelectionViewer::setAxialPlaneDisplay(bool display)
{
    this->displayingAxialPlane = display;
    if (display)
        axialPlaneMesh.show();
    else
        axialPlaneMesh.hide();
    this->update();
    return display;
}

bool ShoulderSelectionViewer::setCoronalPlaneDisplay(bool display)
{
    this->displayingCoronalPlane = display;
    if (display)
        coronalPlaneMesh.show();
    else
        coronalPlaneMesh.hide();
    this->update();
    return display;
}

bool ShoulderSelectionViewer::setFriedmanAxisDisplay(bool display)
{
    this->displayingFriedmanAxis = display;
    if (display)
        friedmanAxisMesh.show();
    else
        friedmanAxisMesh.hide();
    this->update();
    return display;
}

void ShoulderSelectionViewer::computeAllPlanes()
{
    this->computeSagitalPlaneMesh();
    this->computeAxialPlaneMesh();
    this->computeCoronalPlaneMesh();
    this->computeFriedmanAxisMesh();
    this->update();
}


void ShoulderSelectionViewer::backToPointSelection()
{
    this->entryPointOnFriedman->hide();

    dirMesh.hide();
    normalMesh.hide();
    binormalMesh.hide();


    this->centre_glene->show();
    this->trigone->show();
    this->scapula->show();

    this->recomputeEntryPointPosition();

    this->update();
}

void ShoulderSelectionViewer::initPlannificationInput()
{
    this->entryPointOnFriedman->setPosition(this->getGlenePosition());
    this->entryPointOnFriedman->show();

    dirMesh.show();
    normalMesh.show();
    binormalMesh.show();

    this->centre_glene->hide();
    this->trigone->hide();
    this->scapula->hide();

    this->recomputeEntryPointPosition();

    this->update();
}

void ShoulderSelectionViewer::setInclinaison(double newInclinaison)
{
    // Use negative value to fit the convention
    this->inclinaison = newInclinaison;
    this->recomputeEntryPointPosition();
}

void ShoulderSelectionViewer::setVersion(double newVersion)
{
    // Use negative value to fit the convention
    this->version = newVersion;
    this->recomputeEntryPointPosition();
}

void ShoulderSelectionViewer::moveEntryPointToFitShoulder()
{
    Vector3 newEntryPointPosition = this->entryPointOnFriedman->getPosition();
    Vector3 fromGleneToEntryPoint = (newEntryPointPosition - getGlenePosition());
    Vector3 rayOrigin = (getGlenePosition() - getGleneToTrigoneVector()) + fromGleneToEntryPoint;
    Vector3 rayDir = getGleneToTrigoneVector() * 10.f;

    Vector3 collisionPoint(false);
    std::vector<std::vector<Vector3>> triangles = this->shoulderMesh.getTriangles();
    for (auto& t : triangles) {
        Vector3 v1 = t[0];
        Vector3 v2 = t[1];
        Vector3 v3 = t[2];
        Vector3 collision = Collision::segmentToTriangleCollision(rayOrigin, rayOrigin + rayDir, v1, v2, v3);
        if (collision.isValid()) {
            if (!collisionPoint.isValid() || (collision - rayOrigin) < (collisionPoint - rayOrigin))
                collisionPoint = collision;
        }
    }
    if (collisionPoint.isValid()) {
        this->entryPointOnFriedman->blockSignals(true);
        this->entryPointOnFriedman->setPosition(collisionPoint);
        this->entryPointOnFriedman->blockSignals(false);
    }
    this->update();
    computePlannificationTransformMatrix();
}

void ShoulderSelectionViewer::recomputeEntryPointPosition()
{
    /*
    // Retrieve the distance to be reused at the end
    float distanceFromCentreGlene = getDistanceToEntryPoint();
    Vector3 vectorFromCentreGlene = getGleneToTrigoneVector().normalized();
    if (distanceFromCentreGlene > 1e-8)
        vectorFromCentreGlene *= -distanceFromCentreGlene;

    // Aliases to get more lisibility
    float versionRad = -degToRad(this->version);
    float inclinaisonRad = -degToRad(this->inclinaison);

    Vector3 versionRotationAxis = getScapulaToFriedmanVector();
    vectorFromCentreGlene.rotate(versionRad, versionRotationAxis.normalized());

    Vector3 inclinaisonRotationAxis = vectorFromCentreGlene.cross(getScapulaToFriedmanVector());
    vectorFromCentreGlene.rotate(inclinaisonRad, inclinaisonRotationAxis);

//    this->entryPointOnFriedman->setPositions(getGlenePosition() + vectorFromCentreGlene, getGlenePosition());
//    this->entryPointOnFriedman->setValue(0.f);
*/
    this->computePlannificationTransformMatrix();

    this->update();
}

void ShoulderSelectionViewer::openStlFile(std::string filename)
{
    this->shoulderMesh.fromStl(filename);
    shoulderMesh.update();

    LightSource light = PositionalLight(
                new float[4]{.5, .5, .5, 1.},
                new float[4]{.8, .8, .8, 1.},
                new float[4]{.5, .5, .5, 1.},
                Vector3(0.0, 0.0, 100.0)
                );
    float globalAmbiant[4] = {.10, .10, .10, 1.0};
    Material bone_material(
                    new float[4] { 20/255.f,  20/255.f,  20/255.f, 1.f}, // new float[4]{.28, .90, .00, 1.},
                    new float[4] {200/255.f, 200/255.f, 200/255.f, 1.f}, // new float[4]{.32, .80, .00, 1.},
                    new float[4] {250/255.f, 250/255.f, 250/255.f, 1.f}, // new float[4]{.62, .56, .37, 1.},
                    51.2f
                    );
    shoulderMesh.shader->setLightSource("light", light);
    shoulderMesh.shader->setVector("globalAmbiant", globalAmbiant, 4);
    shoulderMesh.shader->setMaterial("material", bone_material);

    Vector3 minAABBox(false);
    Vector3 maxAABBox(false);
    Vector3 center;
    for(Vector3& vert : shoulderMesh.vertexArray) {
        if (!minAABBox.isValid()) minAABBox = vert;
        if (!maxAABBox.isValid()) maxAABBox = vert;
        minAABBox = Vector3(std::min(minAABBox.x, vert.x), std::min(minAABBox.y, vert.y), std::min(minAABBox.z, vert.z));
        maxAABBox = Vector3(std::max(maxAABBox.x, vert.x), std::max(maxAABBox.y, vert.y), std::max(maxAABBox.z, vert.z));
        center += vert;
    }
    center /= (float)shoulderMesh.vertexArray.size();
//    std::cout << "Center at " << center << " - from " << minAABBox << " to " << maxAABBox << std::endl;
    this->setSceneCenter(center);
    this->setSceneRadius((maxAABBox - minAABBox).norm());
    this->camera()->setPosition(minAABBox - (maxAABBox - minAABBox));
    this->camera()->lookAt(center);
}






QString ShoulderSelectionViewer::helpString() const {
  QString text("<h2>Planification Epaule</h2>");
  text += "Cette application minimaliste vous permet de définir la direction de la broche guide "
          "";
  return text;
}
