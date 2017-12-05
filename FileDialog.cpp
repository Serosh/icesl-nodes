#ifndef EMSCRIPTEN

#ifndef WIN32
#include <QApplication>
#include <QFileDialog>
#endif

#include <LibSL/LibSL.h>

#include "FileDialog.h"

#ifndef WIN32

static QApplication *s_QtApp = NULL;
static int    s_Argc = 0;
static char **s_Argv = NULL;

/// ==============================  Linux  =================================

std::string openFileDialog(std::string extension)
{
  if ( s_QtApp == NULL ) {
    s_QtApp = new QApplication(s_Argc,s_Argv);
    // Qt likes to reset the locale when creating a QApplication... WTF
    std::locale::global(std::locale("C"));
   }
  // FC: use QFileDialog::DontUseNativeDialog as otherwise the file dialog won't close
  // http://qt-project.org/forums/viewthread/34159
  //
  QString str = QFileDialog::getOpenFileName(NULL, "Open File", "./", extension.c_str(), NULL, QFileDialog::DontUseNativeDialog);
  std::locale::global(std::locale("C"));
  //std::cerr << "LC_ALL: " << setlocale(LC_ALL, NULL) << std::endl;
  if (str.isNull()) {
    return "";
  } else {
    return str.toStdString();
  }
}

std::string openPathDialog()
{
  if ( s_QtApp == NULL ) {
    s_QtApp = new QApplication(s_Argc,s_Argv);
    // Qt likes to reset the locale when creating a QApplication... WTF
    std::locale::global(std::locale("C"));
   }
  QString path = QFileDialog::getExistingDirectory(NULL, "Select folder", NULL, QFileDialog::DontUseNativeDialog);
  std::locale::global(std::locale("C"));


  //std::cerr << "LC_ALL: " << setlocale(LC_ALL, NULL) << std::endl;
  if (path.isNull()) {
    return "";
  } else {
    return path.toStdString();
  }
}
std::string saveFileDialog(std::string proposedFileNameFullPath)
{
  if ( s_QtApp == NULL ) {
    s_QtApp = new QApplication(s_Argc,s_Argv);
    // Qt likes to reset the locale when creating a QApplication... WTF
    std::locale::global(std::locale("C"));
  }
  // FC: use QFileDialog::DontUseNativeDialog as otherwise the file dialog won't close
  // http://qt-project.org/forums/viewthread/34159
  QString str = QFileDialog::getSaveFileName(NULL, ("Save File"), proposedFileNameFullPath.c_str(), ("Any (*.*)"), NULL, QFileDialog::DontUseNativeDialog);
  std::locale::global(std::locale("C"));
  //std::cerr << "LC_ALL: " << setlocale(LC_ALL, NULL) << std::endl;
  if (str.isNull()) {
    return "";
  } else {
    return str.toStdString();
  }
}

#else

#include <Windows.h>

using namespace std;

/// ============================== Windows =================================

/** Function that open a file 
    @param extention: file extention
*/
std::string openFileDialog(std::string extension)
{
  char current[1024];
  GetCurrentDirectoryA(1024, current); //Retrieve the current directory
  char szFile[512];
  memset(szFile,0x00,512);
  OPENFILENAMEA of;  //of is the name given to the file 
  ZeroMemory(&of, sizeof(of));
  of.lStructSize = sizeof(of);// The length, in bytes of the file 
  of.hwndOwner = NULL; // SimpleUI::getHWND();
  of.lpstrFile = szFile;
  // Set lpstrFile[0] to '\0' so that GetOpenFileName does not
  // use the contents of szFile to initialize itself.
  of.lpstrFile[0] = '\0';
  of.nMaxFile = sizeof(szFile); // storage variable for path and file name
  of.lpstrFilter = "All\0*.*\0Script\0*.lua\0STL\0*.stl\0OBJ\0*.obj\0IceSL\0*.ice\0";
  of.nFilterIndex = 1;
  of.lpstrFileTitle = NULL;//The file name and extension (without path information) of the selected file
  of.nMaxFileTitle = 0;
  of.lpstrInitialDir = NULL;
  of.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;//A set of bit flags to use to initialize the dialog box
  //OFN_PATHMUSTEXIST: The user can type only valid paths and file names
  //OFN_FILEMUSTEXIST:The user can type only names of existing files in the File Name entry field
  if (GetOpenFileNameA(&of)) {  //Open the file
    string fname = string(of.lpstrFile);
		SetCurrentDirectoryA(current);
    cerr << "DIRECTORY (openfile) : " << current << endl;
    return fname;
  }
  SetCurrentDirectoryA(current);
  return "";
}

// Function that open an existing file using the path  

std::string openPathDialog() 
{
  char current[1024];
  GetCurrentDirectoryA(1024, current); 
	char szFile[512]; 
	memset(szFile, 0x00, 512);
	OPENFILENAMEA of; 
	ZeroMemory(&of, sizeof(of));
	of.lStructSize = sizeof(of); 
	of.hwndOwner = NULL; // SimpleUI::getHWND();
	of.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
	// use the contents of szFile to initialize itself.
	of.lpstrFile[0] = '\0';
	of.nMaxFile = sizeof(szFile); 
	of.lpstrFilter = "All\0*.*\0";
	of.nFilterIndex = 1;      
	of.lpstrFileTitle = NULL; 
	of.nMaxFileTitle = 0; 
	of.lpstrInitialDir = NULL;
	of.Flags = OFN_PATHMUSTEXIST; //OFN_PATHMUSTEXIST: The user can type only valid paths and file names
	if (GetOpenFileNameA(&of)) {
    SetCurrentDirectoryA(current);  //Change directory to the current one
    string fname = string(of.lpstrFile);
		std::string directory;
		const size_t last_slash_idx = fname.rfind('\\');
		if (std::string::npos != last_slash_idx) {
      directory = fname.substr(0, last_slash_idx);
      return directory;
		}
		return fname;
	}
  SetCurrentDirectoryA(current);
  return "";
}

// Function that save a file

std::string saveFileDialog(std::string proposedFileNameFullPath)
{
  char szFile[MAX_PATH]; //The size of the path 
  std::strcpy(szFile, proposedFileNameFullPath.c_str()); //Put in the szfile variable the path of the proposed file
  OPENFILENAMEA of;
  ZeroMemory(&of, sizeof(of));
  of.lStructSize = sizeof(of);
  of.hwndOwner = NULL;
  of.lpstrFile = szFile;
  of.nMaxFile = sizeof(szFile);
  of.lpstrFilter = "All\0*.*\0";
  of.nFilterIndex = 1;
  of.lpstrFileTitle = NULL;
  of.nMaxFileTitle = 0;
  of.lpstrInitialDir = NULL;
  of.Flags = OFN_HIDEREADONLY; //OFN_HIDEREADONLY: Hides the Read Only check box
  if (GetSaveFileNameA(&of)) { //Creates a Save dialog box that lets the user specify the drive, directory, and name of a file to save
    string fname = string(of.lpstrFile);
    return fname;
  }
  return "";
}

#endif

#endif

/// =========================================================================

