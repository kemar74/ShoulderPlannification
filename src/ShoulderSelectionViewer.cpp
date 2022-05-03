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
    this->setBackgroundColor(QColor(Qt::GlobalColor::white));
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

    if (useWireframeMode){
//        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
//        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    Material bone_material(
                    new float[4] {204/255.f, 198/255.f, 181/255.f, (useWireframeMode && false ? .5f : 1.f)}, // new float[4]{.28, .90, .00, 1.},
                    new float[4] {102/255.f,  99/255.f,  90/255.f, (useWireframeMode && false ? .5f : 1.f)}, // new float[4]{.32, .80, .00, 1.},
                    new float[4] {  0/255.f,   0/255.f,   0/255.f, (useWireframeMode && false ? .5f : 1.f)}, // new float[4]{.62, .56, .37, 1.},
                    51.2f
                    );
    shoulderMesh.shader->setMaterial("material", bone_material);

    this->shoulderMesh.display();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (useWireframeMode) {
        glClear(GL_DEPTH_BUFFER_BIT);
    }

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

void ShoulderSelectionViewer::mouseDoubleClickEvent(QMouseEvent *event)
{
//    QGLViewer::mouseDoubleClickEvent(event);
    if (this->centre_glene_validated && this->trigone_validated && this->scapula_validated)
    {
        float maxDistance = (this->camera()->position() - this->camera()->pivotPoint()).norm();
        Vector3 xAxis = getScapulaToFriedmanBivector().normalize();
        Vector3 yAxis = getGleneToTrigoneVector().normalize();
        Vector3 camDir = this->camera()->viewDirection();

        float xDot = camDir.dot(xAxis);
        float yDot = camDir.dot(yAxis);

        if (std::abs(xDot) > std::abs(yDot)) {
            this->camera()->setViewDirection(xAxis * (xDot > 0.f ? 1.f : -1.f));
        } else {
            this->camera()->setViewDirection(yAxis * (yDot > 0.f ? 1.f : -1.f));
        }
        this->camera()->setUpVector(getScapulaToFriedmanVector());
        this->camera()->setPosition(getCentralPosition() - Vector3(camera()->viewDirection()) * maxDistance);

        this->update();
    } else {
        QGLViewer::mouseDoubleClickEvent(event);
    }
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


    float versionRad = -degToRad(this->version);
    float inclinaisonRad = degToRad(this->inclinaison);
    float rotationRad = -degToRad(this->rotation);

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

    Vector3 rotationRotationAxis = plannifDirVector;
    plannifNormalVector.rotate(rotationRad, rotationRotationAxis);
    plannifBinormalVector.rotate(rotationRad, rotationRotationAxis);

    Vector3 displayedEntryPoint = getGlenePosition() + vectorFromCentreGlene.normalized() * -getDistanceToEntryPoint() + fromGleneToEntry;

    dirMesh.fromArray({displayedEntryPoint, displayedEntryPoint + plannifDirVector.rotated(-PI/2.f, plannifBinormalVector) * 100.f});
    normalMesh.fromArray({displayedEntryPoint, displayedEntryPoint + plannifNormalVector.rotated(-PI/2.f, plannifBinormalVector) * 100.f});
    binormalMesh.fromArray({displayedEntryPoint, displayedEntryPoint + plannifBinormalVector * 100.f});

    std::vector<float> matrix_data = {
        plannifDirVector.x     , plannifDirVector.y     , plannifDirVector.z     , 0,
        plannifBinormalVector.x, plannifBinormalVector.y, plannifBinormalVector.z, 0,
        plannifNormalVector.x  , plannifNormalVector.y  , plannifNormalVector.z  , 0,
        entryPoint.x           , entryPoint.y           , entryPoint.z           , 1
    };

    Matrix transform(4, 4, matrix_data.data());
    std::cout << transform << std::endl;

    std::vector<GLdouble> d_matrix_data;
    for (const auto& data : matrix_data) {
        d_matrix_data.push_back(data);
    }
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

void ShoulderSelectionViewer::setRotation(double newRotation)
{
    this->rotation = newRotation;
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

    this->setSceneCenter(center);
    this->setSceneRadius((maxAABBox - minAABBox).norm() * .8f);

    this->camera()->lookAt(center);
    this->showEntireScene();

    float controlPointsRadius = (maxAABBox - minAABBox).norm() * 0.01f;
    std::cout << controlPointsRadius << std::endl;
    this->centre_glene->setSphereRadius(controlPointsRadius);
    this->trigone->setSphereRadius(controlPointsRadius);
    this->scapula->setSphereRadius(controlPointsRadius);

    float ambiant[4]  = {.3, .3, .3, 1.};
    float diffuse[4]  = {.5, .5, .5, 1.};
    float specular[4] = {.1, .1, .1, 1.};

    LightSource light0 = PositionalLight(
                ambiant,
                diffuse,
                specular,
                center + Vector3(0.0, 0.0, 100.0)
                );
    LightSource light1 = PositionalLight(
                ambiant,
                diffuse,
                specular,
                center + Vector3(0.0, 100.0, 0.0)
                );

    float globalAmbiant[4] = {.10, .10, .10, 1.0};
    shoulderMesh.shader->clearLightSources("lights");
    shoulderMesh.shader->addLightSource("lights", light0);
    shoulderMesh.shader->addLightSource("lights", light1);
    shoulderMesh.shader->setVector("globalAmbiant", globalAmbiant, 4);
}






QString ShoulderSelectionViewer::helpString() const {
  QString text("<h2>Planification Epaule</h2>");
  text += "Cette application minimaliste vous permet de définir la direction de la broche guide "
          "pour une plannification à faire. <br>"
          "En entrée, importez un modèle 3D de l'omoplate au format STL puis en sortie vous retrouverez "
          "une matrice de transformation afin de configurer vos axes programmés. <br>"
          "<h3>Description de l'interface</h3>"
          "En ouvrant le programme, un unique bouton est affiché dans la partie droite : \"Chargez un modèle STL pour commencer\".<br>"
          "En cliquant sur ce bouton, vous pourrez chercher sur votre ordinateur pour le modèle 3D qui vous interesse. <br>"
          "Vous avez alors une interface découpée en deux parties : <br>"
          "<ul><li>À gauche : Une visualisation de l'os. Vous pouvez tourner autour en maintenant un clic gauche de la souris, "
          "se translater en maintenant le clic droit et zoomer/dézoomer avec la molette de la souris.</li>"
          "<li>À droite : Une série de boutons et d'options pas très esthétiques découpées en 3 niveaux :"
          "<ul><li>En premier, la \"definition de points\" vous permettra de définir les points d'interêt sur le modèle 3D"
          "(centre glène, trigone et base de la scapula). <br>"
          "En selectionnant par exemple \"Centre glène\", vous pouvez définir le centre glène en cliquant sur le modèle 3D.</li>"
          "<li>En second, quand les 3 points d'interêt seront définis, vous pourrez passer à la plannification en cliquant sur "
          "la case à cocher \"Plannification\". La partie supérieure devrait se griser et cette partie se dégrisera. <br>"
          "Dans la visualisation 3D un repère rouge vert et bleu s'affichera au point du centre glène, avec un petit point brun. <br>"
          "Cela correspond à votre point d'entrée. En modifiant les valeurs \"Inclinaison (°)\", \"Version (°)\" et \"Rotation (°)\""
          "vous verrez le repère se modifier pour conformer aux besoins. Vous pouvez déplacer le point d'entrée sur la surface de l'os "
          "en cliquant sur le point brun. (Tips : utilisez <em>possiblement très doucement</em> la molette de la souris sur"
          "le point pour augmenter/réduire sa taille). <br>"
          "Quand votre configuration est bonne, cliquez sur \"Exporter le résultat\" pour obtenir toutes les informations à utiliser "
          "dans vos prochains programmes. </li>"
          "<li>Enfin, vous pouvez cocher/décocher les cases de \"Affichage\" pour voir/cacher les plans et axes de l'os. <br>"
          "Cliquez sur \"Afficher en transparence\" pour pouvoir voir les axes à travers la surface de l'os. </li>"
          "</ul>"
          "</li>"
          "</ul>"
          ""
          "<h3>Manipulation de l'interface</h3>"
          "Tournez-vous autour de l'os en maintenant un clic gauche de la souris, "
          "déplacez-vous en maintenant le clic droit et zoomez/dézoomez avec la molette de la souris. <br>"
          "Les points d'interêt et le point d'entrée peuvent être déplacés en mantenant cliquée et en déplacant la souris. <br>"
          "Le point d'entrée restera à la surface de l'os. <br>"
          "Si les points sont trop grands/trop petits, utilisez la molette de la souris (ou deux doigts sur le trackpad) pour "
          "agrandir/réduire leur taille. </br>"
          ""
          "<h3>Sortie de l'application</h3>"
          "En cliquant sur \"Exporter les résultats\", le point d'entrée et les paramètres d'angles seront traduit en une "
          "\"matrice de transformation\". Cette matrice est généralement utilisée par d'autres logiciels de plannification. <br>"
          "Une fenêtre s'ouvrira en vous présentant cette matrice sous plusieurs formes (selon le logiciel necessitant cette matrice). <br>"
          "<ul><li> Premièrement, la matrice sous forme \"mathématique\".</li>"
          "<li>Les versions \"column-major\" et \"row-major\" vous sont données. Elles représentent cette même matrice, mais en une "
          "unique ligne, ce qui est souvent demandé en entrée par les logiciels 3D. Une version parcourt la matrice de haut en bas "
          "(column-major) tandis que l'autre la parcourt de gauche à droite (row-major). Simplement selectionnez et copiez-collez "
          "le format qui vous interesse. </li>"
          "<li>Vous retrouverez aussi les coordonnées du point d'entrée. Ces coordonnées sont contenues dans la matrice précédente, "
          "mais vous aurez peut-être besoin d'avoir uniquement cette information. </li>"
          "</ul>"
          ""
          "<h3>Plus d'informations / Questions </h3>"
          "N'hesitez pas à m'envoyer un mail (marc.hartley@lirmm.fr)";
  return text;
}
