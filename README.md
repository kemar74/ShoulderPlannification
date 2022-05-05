# ShoulerPlannification
------
Application minimaliste pour la plannification d'opérations de l'omoplate.

## Objectif
Ce logiciel vous permet facilement de charger un modèle 3D au format STL et de calculer semi-automatiquement les plans principaux de l'épaule.

## Configuration necessaire
Le programme est developé sous Qt 5.15.2 sous Windows11.  
Il a été testé :  
- Sur Windows11 avec Processeur AMD Ryzen 7 4800H with Radeon Graphics 2.90 GHz, 16Go RAM, CG NVIDIA GeForce GTX 1650 Ti/PCIe/SSE2 et OpenGL 4.6.0 en installant libQGLViewer 2.7.2 et glew 2.1.0
- Sur Ubuntu 18.04.6 LTS avec Processeur Intel Core i7-9700K CPU 3.60 GHz x 8, 16Go RAM, CG GeForce RTX 2080/PCIe/SSE2 et OpenGL 4.6.0 en installant libQGLViewer 2.7.2 et glew 2.0.0
- Sur MacOS Catalina 10.15.7 avec Processeur 2.2 GHz Intel Core i7 6 coeurs, 16Go RAM, CG Radeon Pro 555X 4Go et OpenGL 2.1 ATI-3.10.22 en installant libQGLViewer 2.8.0 et glew 2.2.0 

Dans la suite du document, je considèrerai probablement les chemins comme s'ils commençaient par "C:\" (version Windows), mais vous l'adapterez selon votre OS.

Vous retrouverez (j'espère) [quelques releases](https://github.com/kemar74/ShoulderPlannification/releases) dans ce repertoire qui peuvent vous permettre de récuperer le logiciel sans le compiler from source.

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
> Déplacez-vous dans la section `unix:mac {}`  
> et reperez la ligne ressemblant à `INCLUDEPATH \*= "/usr/local/opt/glew/include"` ainsi que `LIBS \*= -L/usr/local/opt/glew/lib` pour adapter le chemin à votre dossier d'installation. 

> Vous pouvez lancer la commande `find / -name glew 2> /dev/null` pour trouver un chemin possible.

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

#### Sous MacOS
Pour être sûrs, commencez par installer Qt :  
Avec Homebrew : `brew install qt5`  
Avec MacPorts : `sudo port install qt5`  

Ensuite on va installer l'IDE QtCreator :  
Accedez au [lien vers l'installateur](https://www.qt.io/download-qt-installer) puis cliquez sur "Download".  
Lancez l'installateur, créez un compte Qt (environ 2 minutes), acceptez les conditions puis appuyez sur "Suivant".  
Choisissez le repertoire "C:\Qt" et l'installation personnalisée. Vous pouvez décocher l'association de type si vous le souhaitez. Appuyez sur "Suivant".  
Parmis les composants à installer, developpez "Qt" et developpez "Qt 5.15.2". Si le but est seulement de compiler ce projet, ne cochez que "macos" et "Sources". 

### Etape 3 : installation de LibQGLViewer
Accedez à [libqglviewer.com](http://libqglviewer.com/download.html), un petit plugin liant Qt et OpenGL.  

* Partie Windows *  
Telechargez le .zip dans "C:\Qt" puis extraire le contenu.  

* Partie MacOS et Linux*  
Telechargez le .tar.gz dans "C:\Qt" puis extraire le contenu.

Depuis QtCreator, ouvrez le fichier extrait "QGLViewer/QGLViewer.pro" (Pas celui à la racine du dossier, mais dans le dossier "QGLViewer"). Cliquez sur "Configure project" en bas à droite.  
La section "Mac" de ce fichier .pro, on va forcer la création de librairies dynamiques. Pour ça, repérez la ligne `!staticlib: CONFIG *= lib_bundle` et commentez-la :  
```# !staticlib: CONFIG *= lib_bundle```  

Puis lancez la compilation avec le "Build" (symbole de marteau en bas à gauche).

* Partie Windows *  
Au même niveau que "QGLViewer.pro" vous devez maintenant trouver les fichiers "QGLViewer2.dll" et "QGLViewer2d.dll". Copiez-les dans le dossier "System32" de votre machine (ou alternativement uniquement uniquement aux cotés des futurs executables créés par la suite).  
Dans notre projet à nous, modifiez la configuration en repérant dans "ShoulderPlannification.pro" les lignes `INCLUDEPATH *= "C:\Qt\libQGLViewer-2.7.2"` et `LIBS *= -L"C:\Qt\libQGLViewer-2.7.2\QGLViewer" -lQGLViewer2 -lOpengl32` et modifiez les pour correspondre à votre cas.

* Partie Linux *  
Au même niveau que "QGLViewer.pro" vous devez maintenant trouver le fichier "libQGLViewer-2.X.X.so". Copiez-les dans le dossier "/usr/lib/" de votre machine (ou alternativement uniquement uniquement aux cotés des futurs executables créés par la suite).  
Dans notre projet à nous, modifiez la configuration en repérant dans "ShoulderPlannification.pro" les lignes `INCLUDEPATH *= /home/simulateurrsm/Documents/libqglviewer/libQGLViewer-2.7.2` et `LIBS *= -L/home/simulateurrsm/Documents/libqglviewer/libQGLViewer-2.7.2\QGLViewer` et modifiez les pour correspondre à votre cas.

* Partie MacOS *
Si vous avez bien compilé la bibliothèque en dynamique, vous devez retrouver 4 fichier ".dylib" dans le dossier "QGLViewer".
> La section "Mac" de ce fichier .pro, on va forcer la création de librairies dynamiques. Pour ça, repérez la ligne `!staticlib: CONFIG *= lib_bundle` et commentez-la :  
```# !staticlib: CONFIG *= lib_bundle```  
Dans notre projet à nous, modifiez la configuration en repérant dans "ShoulderPlannification.pro" les lignes `INCLUDEPATH *= /Users/cyril/Desktop/marc/Qt/libQGLViewer-2.8.0/` et `LIBS *= -L/Users/cyril/Desktop/marc/Qt/libQGLViewer-2.8.0/QGLViewer` et modifiez les pour correspondre à votre cas.

#### (Impossible de lancer QtCreator?)
Si les projets ne peuvent pas être configurés par QtCreator, on va devoir faire ça à la main. Qt devrait tout de même avoir installé les compilateurs et outils de compilation essentiels.  
Les outils de compilation sont installés dans un repertoire du genre "chemin/vers/Qt/5.15.2/nom_de_mon_compilateur/bin/". (Sous windows, "nom_de_mon_compilateur" peut être "minGW_81_64" par exemple, ou sur MacOS ça peut être "clang", etc...)

Ouvrez un terminal et positionnez-vous au niveau du fichier .pro qui vous interesse.
Lancez la compilation : `chemin/vers/Qt/5.15.2/nom_de_mon_compilateur/bin/qmake`  
Sous MacOS, il faut ajouter des options: `chemin/vers/Qt/5.15.2/nom_de_mon_compilateur/bin/qmake -spec macx-xcode`  

Cela génère un Makefile, vous pouvez maintenant lancer la simple commande `make`.



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
copy ShoulderPlannification.exe "C:\chemin\vers\ShoulderPlannification\ShoulderPlannification.exe"
```

Vous pouvez maintenant lancer l'application depuis le .exe créé à la racine car il sera accompagné de plusieurs ".dll" et dossiers de Qt. N'hésitez pas à créer un raccourci pour lancer l'application depuis un endroit plus pratique.

#### Sous MacOS 
(Il me semble que c'est assez identique à Windows)

Créez un vrai executable .app avec l'outil fourni par Qt:  
Par la dernière compilation, vous avez créé un executable "lié" à QtCreator. Il doit se trouver dans un dossier "build-Debug\debug" ou "build-Release\release". Il ne peut pas être lancé tout seul, mais on va corriger ça:  

Dans un invité de commande:  
```
cd \chemin\vers\ShoulderPlannification\puis\le\dossier\avec\le\app\cree
macdeployqt ShoulderPlannification.app
cp chemin/vers/libQGLViewer/QGLViewer/libQGLViewer.2.dylib Contents/Frameworks/libQGLViewer.2.dylib
install_name_tool -change libQGLViewer.2.dylib @executable_path/../Frameworks/libQGLViewer.2.dylib ShoulderPlannification/Contents/MacOS/ShoulderPlannification
macdeployqt ShoulderPlannification.app
```
L'outil "macdeployqt" n'est peut-être pas inclu dans le PATH, donc il faudra possiblement le retrouver dans le dossier d'installation de Qt.  
Le chemin est probablement du genre "chemin/vers/Qt/5.15.2/clang/bin/macdeployqt.app".

Vous pouvez maintenant lancer l'application depuis le .app créé à la racine car il sera accompagné de plusieurs fichiers et dossiers de Qt. N'hésitez pas à créer un raccourci pour lancer l'application depuis un endroit plus pratique.

Pour lancer l'application sans QtCreator, je ne connais pas bien MacOS, mais la commande `ShoulderPlannification.app/Contents/MacOS/ShoulderPlannification` semble fonctionner.


#### Sous Linux 
L'executable créé à la compilation est déjà un vrai executable, rien à faire!


## Compatibilité avec OpenGL moderne
Il est possible que le programme plante dans le cas où la version "moderne" d'OpenGL n'est pas compatible avec votre ordinateur.  
Une petite tentative de corriger ce problème a été initiée, mais je ne garantie rien...  

Si jamais, allez tout en haut du fichier "src/Utils/Globals.h" et modifiez la variable "useModernOpenGL" à "0" (ou à "1"). Puis recompilez le programme.  
Si cela ne répare pas les soucis, merci de me le faire remonter, et pourquoi pas tenter de trouver la solution ensemble.

## En cas de problème
N'hésitez pas à me contacter (marc.hartley@lirmm.fr)