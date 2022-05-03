# ShoulerPlannification
------
Application minimaliste pour la plannification d'opérations de l'omoplate.

## Objectif
Ce logiciel vous permet facilement de charger un modèle 3D au format STL et de calculer semi-automatiquement les plans principaux de l'épaule.

## Configuration necessaire
Le programme est developé sous Qt 5.15.2 sous Windows11.  
Il a été testé avec OpenGL 4.6, libQGLViewer 2.7.2 et glew 2.1.0 sur Windows 11 ainsi que sur Ubuntu 18.04.6 LTS

## Installer le logiciel
Clonez le contenu de ce repertoire git :  
```
git clone https://github.com/kemar74/ShoulderPlannification
cd ShoulderPlannification
```

### Etape 1 : installation de Glew
#### Sous Windows
Installez Glew en suivant les instruction d'installation dans [le repertoire git de Glew](https://github.com/nigels-com/glew).  
Pour éviter tout souci de compatibilité, je conseille d'utiliser l'archive [glew v2.1.0](https://github.com/nigels-com/glew/releases/tag/glew-2.1.0)
Extraire le contenu du .zip . Vous devrez probablement modifier le fichier de configuration de notre code en entrant dans le fichier "ShoulderPlannification.pro".
et reperez la ligne ressemblant à `INCLUDEPATH *= "C:\codes\CPP\glew-2.1.0\include"` ainsi que `LIBS *= -L"C:\codes\CPP\glew-2.1.0\lib\Release\x64\glew32.lib"` pour adapter le chemin à votre dossier d'installation.
#### Sous MacOS
Avec Homebrew : ```brew install glew```  
Avec MacPorts : ```sudo port install glew +universal``` (peut-être)...  
Sinon, suivez les mêmes instructions que pour Windows :  
> Pour éviter tout souci de compatibilité, je conseille d'utiliser l'archive [glew v2.1.0](https://github.com/nigels-com/glew/releases/tag/glew-2.1.0)
> Extraire le contenu du .zip . Vous devrez probablement modifier le fichier de configuration de notre code en entrant dans le fichier "ShoulderPlannification.pro".
> et reperez la ligne ressemblant à `INCLUDEPATH \*= "C:\codes\CPP\glew-2.1.0\include"` ainsi que `LIBS \*= -L"C:\codes\CPP\glew-2.1.0\lib\Release\x64\glew32.lib"` pour adapter le chemin à votre dossier d'installation.
#### Sous Linux
Sous Ubuntu : 
```sudo apt-get update -y
sudo apt-get install -y libglew-dev
```

### Etape 2 : installation de Qt
#### Sous Windows
Au plus simple, commencez par installer QtCreator : [lien vers l'installateur](https://www.qt.io/download-qt-installer) puis cliquez sur "Download".  
Lancez l'installateur, créez un compte Qt (environ 2 minutes), acceptez les conditions puis appuyez sur "Suivant".  
Choisissez le repertoire "C:\Qt" et l'installation personnalisée. Vous pouvez décocher l'association de type si vous le souhaitez. Appuyez sur "Suivant".  
Parmis les composants à installer, developpez "Qt" et developpez "Qt 5.15.2". Si le but est seulement de compiler ce projet, ne cochez que "MinGW X.X.X 32/64 bits". (Qui pèse déjà 6Go sur Windows...)  

#### Sous Linux 
Il semblerait qu'il est possible d'installer Qt et QtCreator avec 
```
sudo apt install qt5-default
sudo apt install build-essential
sudo apt install qtcreator
```
En espérant que ce soit aussi simple sur MacOS...

### Etape 3 : installation de LibQGLViewer
Accedez à [libqglviewer.com](http://libqglviewer.com/download.html), un petit plugin liant Qt et OpenGL.  

* Partie Windows *  
Telechargez le .zip dans "C:\Qt" puis extraire le contenu.  

* Partie MacOS et Linux*  
Telechargez le .tar.gz dans "C:\Qt" puis extraire le contenu.

Depuis QtCreator, ouvrez le fichier extrait "QGLViewer/QGLViewer.pro". Cliquez sur "Configure project" en bas à droite. Puis lancez la compilation avec le "Build" (symbole de marteau en bas à gauche).

* Partie Windows *  
Au même niveau que "QGLViewer.pro" vous devez maintenant trouver les fichiers "QGLViewer2.dll" et "QGLViewer2d.dll". Copiez-les dans le dossier "System32" de votre machine (ou alternativement uniquement uniquement aux cotés des futurs executables créés par la suite).  
Dans notre projet à nous, modifiez la configuration en repérant dans "ShoulderPlannification.pro" les lignes `INCLUDEPATH *= "C:\Qt\libQGLViewer-2.7.2"` et `LIBS *= -L"C:\Qt\libQGLViewer-2.7.2\QGLViewer" -lQGLViewer2 -lOpengl32` et modifiez les pour correspondre à votre cas.

* Partie MacOS et Linux *  
Au même niveau que "QGLViewer.pro" vous devez maintenant trouver le fichier "libQGLViewer-2.X.X.so". Copiez-les dans le dossier "/usr/lib/" de votre machine (ou alternativement uniquement uniquement aux cotés des futurs executables créés par la suite).  
Dans notre projet à nous, modifiez la configuration en repérant dans "ShoulderPlannification.pro" les lignes `INCLUDEPATH *= "C:\Qt\libQGLViewer-2.7.2"` et `LIBS *= -L"C:\Qt\libQGLViewer-2.7.2\QGLViewer" -lQGLViewer2 -lOpengl32` et modifiez les pour correspondre à votre cas.



### Etape 4 : enfin lancer notre programme!
Depuis QtCreator, retrouvez dans notre dossier initial le fichier "ShoulderPlannification.pro".  
Compilez et lancez le programme (symbole "Build&Run" en triangle vert en bas à gauche). Si aucune erreur de compilation ou d'execution, nickel!  


#### Sous Windows
Créez un vrai executable .exe avec l'outil fourni par Qt:  
Par la dernière compilation, vous avez créé un executable "lié" à QtCreator. Il doit se trouver dans un dossier "build-Debug\debug" ou "build-Release\release". Il ne peut pas être lancé tout seul, mais on va corriger ça:  

Dans un invité de commande:  
```
C:\Qt\5.15.2\mingw81_64\bin\qtenv2.bat
cd C:\chemin\vers\ShoulderPlannification\puis\le\dossier\avec\le\exe\cree
windeployqt --no-quick-import --no-translations --no-system-d3d-compiler --dir "C:\chemin\vers\ShoulderPlannification" ShoulderPlannification.exe
cp ShoulderPlannification.exe "C:\chemin\vers\ShoulderPlannification\ShoulderPlannification.exe"
```

Vous pouvez maintenant lancer l'application depuis le .exe créé à la racine car il sera accompagné de plusieurs ".dll" et dossiers de Qt. N'hésitez pas à créer un raccourci pour lancer l'application depuis un endroit plus pratique.

#### Sous MacOS 
(Il me semble que c'est assez identique à Windows)

Créez un vrai executable .app avec l'outil fourni par Qt:  
Par la dernière compilation, vous avez créé un executable "lié" à QtCreator. Il doit se trouver dans un dossier "build-Debug\debug" ou "build-Release\release". Il ne peut pas être lancé tout seul, mais on va corriger ça:  

Dans un invité de commande:  
```
cd C:\chemin\vers\ShoulderPlannification\puis\le\dossier\avec\le\app\cree
macdeployqt ShoulderPlannification.app
cp ShoulderPlannification.app "C:\chemin\vers\ShoulderPlannification\ShoulderPlannification.app"
```

Vous pouvez maintenant lancer l'application depuis le .app créé à la racine car il sera accompagné de plusieurs fichiers et dossiers de Qt. N'hésitez pas à créer un raccourci pour lancer l'application depuis un endroit plus pratique.

#### Sous Linux 
L'executable créé à la compilation est déjà un vrai executable, rien à faire!


## Compatibilité avec OpenGL moderne
Il est possible que le programme plante dans le cas où la version "moderne" d'OpenGL n'est pas compatible avec votre ordinateur.  
Une petite tentative de corriger ce problème a été initiée, mais je ne garantie rien...  

Si jamais, allez tout en bas du fichier "src/Graphics/Mesh.cpp" et modifiez la variable "useModernOpenGL" à "false". Puis recompilez le programme.  
Si cela ne répare pas les soucis, merci de me le faire remonter, et pourquoi pas tenter de trouver la solution ensemble.

## En cas de problème
N'hésitez pas à me contacter (marc.hartley@lirmm.fr)