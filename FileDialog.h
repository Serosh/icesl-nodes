#pragma once

#include <string>

/** Declaration of functions used in the FileDialog.cpp **/

std::string openFileDialog(std::string extension);
std::string openPathDialog();
std::string saveFileDialog(std::string proposedFileNameFullPath);


