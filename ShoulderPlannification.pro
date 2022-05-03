QT *= quick opengl xml widgets gui
CONFIG += qt opengl warn_on thread rtti console embed_manifest_exe no_keywords

INCLUDEPATH *= src/

unix {
    INCLUDEPATH *= /home/simulateurrsm/Documents/libqglviewer/libQGLViewer-2.7.2
    LIBS *= -L/home/simulateurrsm/Documents/libqglviewer/libQGLViewer-2.7.2/QGLViewer -lQGLViewer-qt5
}
win32 {
    # I installed the sources of:
    # glew : https://github.com/nigels-com/glew
    # libqglviewer : http://www.libqglviewer.com/src/libQGLViewer-2.7.2.zip
    # Paths might need to be changed for you
    INCLUDEPATH *= "C:\codes\CPP\glew-2.1.0\include"
    INCLUDEPATH *= "C:\Qt\libQGLViewer-2.7.2"

    LIBS *= -L"C:\codes\CPP\glew-2.1.0\lib\Release\x64\glew32.lib"
    LIBS *= -L"C:\Qt\libQGLViewer-2.7.2\QGLViewer" -lQGLViewer2 -lOpengl32
}
CONFIG += c++17

TARGET = ShoulderPlannification

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/DataStructure/Matrix.cpp \
    src/DataStructure/Matrix3.cpp \
    src/DataStructure/Vector3.cpp \
    src/Graphics/Mesh.cpp \
    src/Graphics/Shader.cpp \
    src/Graphics/ShaderElement.cpp \
    src/Graphics/Sphere.cpp \
    src/Interface/ControlPoint.cpp \
    src/Interface/CustomInteractiveObject.cpp \
    src/Interface/FancySlider.cpp \
    src/Interface/InteractiveVector.cpp \
    src/Interface/InterfaceUtils.cpp \
    src/Interface/RangeSlider.cpp \
    src/Interface/Slider3D.cpp \
    src/Interface/Spoiler.cpp \
    src/Interface/StickyFrame.cpp \
    src/Utils/BSpline.cpp \
    src/Utils/Collisions.cpp \
    src/Utils/Globals.cpp \
    src/Utils/Utils.cpp \
    src/ShoulderSelectionViewer.cpp \
    src/ShoulderSelectionWindow.cpp \
    src/main.cpp

HEADERS += \
    src/DataStructure/Matrix.h \
    src/DataStructure/Matrix3.h \
    src/DataStructure/Vector3.h \
    src/Graphics/Mesh.h \
    src/Graphics/Shader.h \
    src/Graphics/ShaderElement.h \
    src/Graphics/Sphere.h \
    src/Interface/ControlPoint.h \
    src/Interface/CustomInteractiveObject.h \
    src/Interface/FancySlider.h \
    src/Interface/InteractiveVector.h \
    src/Interface/InterfaceUtils.h \
    src/Interface/RangeSlider.h \
    src/Interface/Slider3D.h \
    src/Interface/Spoiler.h \
    src/Interface/StickyFrame.h \
    src/Utils/BSpline.h \
    src/Utils/Collisions.h \
    src/Utils/FastNoiseLit.h \
    src/Utils/Globals.h \
    src/Utils/Utils.h \
    src/Utils/stb_image.h \
    src/Utils/stb_image_write.h \
    src/Utils/stl_reader.h \
    src/ShoulderSelectionViewer.h \
    src/ShoulderSelectionWindow.h

FORMS += \
    src/ShoulderSelectionWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    src/Shaders/MarchingCubes_fragment.glsl \
    src/Shaders/MarchingCubes_geometry.glsl \
    src/Shaders/MarchingCubes_vertex.glsl \
    src/Shaders/fragment_shader_gouraud.glsl \
    src/Shaders/grabber_fragment_shader.glsl \
    src/Shaders/grabber_vertex_shader.glsl \
    src/Shaders/grid_fragment_shader_blinn_phong.glsl \
    src/Shaders/grid_vertex_shader_blinn_phong.glsl \
    src/Shaders/layer_based_fragment_shader.glsl \
    src/Shaders/layer_based_vertex_shader.glsl \
    src/Shaders/no_fragment_shader.glsl \
    src/Shaders/no_vertex_shader.glsl \
    src/Shaders/noise.glsl \
    src/Shaders/particle_fragment_shader.glsl \
    src/Shaders/particle_vertex_shader.glsl \
    src/Shaders/vertex_shader_gouraud.glsl \
    src/Shaders/voxels_fragment_shader_blinn_phong.glsl \
    src/Shaders/voxels_vertex_shader_blinn_phong.glsl
