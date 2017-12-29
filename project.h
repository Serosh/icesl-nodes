
#pragma once

#include "imgui/imgui.h"
#include <iostream>

#include <LibSL/LibSL.h>
#ifndef WIN32
#include <sys/types.h>
#include <boost/filesystem.hpp>
#endif

#ifdef WIN32
#include <dirent.h>
#include "Windows.h"
#endif

class Project
{
public:
  std::string path;
  std::set<std::string> openFolders;

#ifndef WIN32
  //---------------------------------------------------
  //trouve le chemin absolu et le copie dans dest_path
  void importLua( std::string path){
    std::string nodeDir = this->path + "/node/";
    boost::filesystem::path src_path(path);
    nodeDir+=  src_path.filename().generic_string();
    boost::filesystem::path dest_path(nodeDir);
    boost::filesystem::copy_file(src_path,dest_path,boost::filesystem::copy_option::overwrite_if_exists);
  }

  //---------------------------------------------------
   //crée un nouveau répertoire pour un nouveau noeud 
  void createNodefolder(){
    std::string NodeDir = path + "/node";
    boost::filesystem::path dir(NodeDir);
    boost::filesystem::detail::create_directory(dir);
  }

  //---------------------------------------------------
  //parcours le directory (vecteur de fichiers)
  //si le fichier n'y es pas --> push_back
  void listLuaFileInDir(std::vector<std::string>& files)
  {
    std::string NodeDir = path + "/node";
    boost::filesystem::path dir(NodeDir);
    for ( boost::filesystem::directory_iterator itr(dir); itr!= boost::filesystem::directory_iterator(); ++itr)
    {
      boost::filesystem::path file = itr->path();
      if(!is_directory(file))files.push_back(file.filename().generic_string());
    }
  }

  //---------------------------------------------------
  // idem precedent, mais prend en argument le directory
  void listLuaFileInDir(std::vector<std::string>& files,std::string directory)
  {
    boost::filesystem::path dir(directory);

    for ( boost::filesystem::directory_iterator itr(dir); itr!= boost::filesystem::directory_iterator(); ++itr)
    {
      boost::filesystem::path file = itr->path();
      if(!is_directory(file))files.push_back(file.filename().generic_string());
    }
  }

  //---------------------------------------------------
  //si le fichier y est on le push back 
  void listFolderinDir(std::vector<std::string>& files, std::string folder){
    boost::filesystem::path dir(folder);

    for ( boost::filesystem::directory_iterator itr(dir); itr!= boost::filesystem::directory_iterator(); ++itr)
    {
      boost::filesystem::path file = itr->path();
      if(is_directory(file))files.push_back(file.filename().generic_string());
    }
  }

  //---------------------------------------------------
  //essaye de créer un dossier de destination et verifie si ok
  //sinon erreur

  //pour chaque fichier dans le directory 
  //essaye de copier sinon erreur

  //renvoie true si copie ok
  bool copyDir(std::string source, std::string destination )
  {
    boost::filesystem::path src(source);
    boost::filesystem::path dest(destination);
    try
    {
      // Create the destination directory
      boost::filesystem::create_directory(dest);
      // Check whether the function call is valid
      if(!boost::filesystem::exists(src) || !boost::filesystem::is_directory(dest)){
        return false;
      }

    }catch(boost::filesystem::filesystem_error const & e){
      std::cerr << e.what() << '\n';
    }
    // Iterate through the source directory
    for(boost::filesystem::directory_iterator file(src);file != boost::filesystem::directory_iterator(); ++file){
      try{
        boost::filesystem::path current(file->path());
        if(boost::filesystem::is_directory(current)){
          std::string next = destination + current.filename().string();
          bool b = copyDir(current.string(),next);
          if(!b){
            return false;
          }
        }else{
          boost::filesystem::copy_file(current.string(),destination +"/"+ current.filename().string());
        }
      }catch(boost::filesystem::filesystem_error const & e){
        std:: cerr << e.what() << '\n';
      }
    }
    return true;
  }

  //---------------------------------------------------
  //comme son nom l'indique
  void exctractPathFromXml(std::string& s){
    boost::filesystem::path p(s.c_str());
    boost::filesystem::path dir = p.parent_path();
    path = dir.generic_string();
  }
#endif
#ifdef WIN32
  //idem tout ce qui est precedement mais si WIN32 est defini
  void importLua(std::string srcPath) 
  {
    std::string nodeDir = this->path + "\\node\\";
    nodeDir += extractFileName(srcPath);
    CopyFile(srcPath.c_str(), nodeDir.c_str(), FALSE);
  }

  void createNodefolder() 
  {
    std::string NodeDir = path + "\\node";
	puts(NodeDir.c_str());
    createDirectory(NodeDir.c_str());
  }

  void listLuaFileInDir(std::vector<std::string>& files) 
  {
    std::string NodeDir = path + "\\node";
    listFiles(NodeDir.c_str(), files);
  }

  void listLuaFileInDir(std::vector<std::string>& files, std::string directory)
  {
    listFiles(directory.c_str(), files);
  }

  void exctractPathFromXml(std::string& s) 
  {
    std::string directory;
    const size_t last_slash_idx = s.rfind('\\');
    if (std::string::npos != last_slash_idx)
    {
      directory = s.substr(0, last_slash_idx);
      path = directory;
    }
  }

  //---------------------------------------------------
  // Not used and not implemented
  void listFolderinDir(std::vector<std::string>& files, std::string folder)
  {

  }

  bool copyFile(std::string filename, std::string dest) {

	  puts(filename.c_str());

	  std::ifstream infile(filename.c_str(), std::ifstream::binary);
	  std::ofstream outfile(dest.c_str(), std::ofstream::binary);

	  infile.seekg(0, infile.end); 

	  long size = infile.tellg(); //size of the infile

	  if (size == -1) {
		  std::cerr << "problem with the copy" << std::endl;
		  return false;
	  }

	  infile.seekg(0); 
	  char* buffer = new char[size];

	  infile.read(buffer, size);
	  outfile.write(buffer, size);

	  outfile.close();
	  infile.close();

	  delete[] buffer;

	  return true;
  }
  //---------------------------------------------------
  //Must copy a Directory source to a directory destination. Using dirent for crossplatform
  bool isDir(struct dirent* file)
  {
	  return ((strchr(file->d_name, '.')) == NULL)? true : false;
  }

  bool copyDir(std::string source, std::string destination)
  {
	DIR * rep = opendir(source.c_str());
	if (rep == NULL) {
		std::cerr << "cannot open : " << source << std::endl;
		return false;
	}

	struct dirent* file = NULL;
	bool isCopied(false);
	while ((file = readdir(rep)) != NULL) {
		puts(file->d_name);
		if (isDir(file)) {
			
			isCopied = copyDir(source + std::string(file->d_name) + std::string("/"), destination);
		}
		else {
			if (strcmp(file->d_name, ".") != 0 && strcmp(file->d_name, "..") != 0) {
				isCopied = copyFile(source +std::string(file->d_name), destination +std::string(file->d_name));
			}
			else isCopied = true;
		}
		if (!isCopied) return false;
	}
	if (closedir(rep) == -1) {
		std::cerr << "cannot close : " << source << std::endl;
		return false;
	}

    return true;
  }
#endif

  //---------------------------------------------------
  // Transform an absolute path to a relative path.
  std::string relativePath(std::string& path) 
  {
    int nfsize = nodefolder().size();
    std::string name = path.substr(nfsize);
    return name;
  }

  //---------------------------------------------------
  // return string of node folder.
  std::string nodefolder()
  {
    std::string NodeDir = path + "\\node\\";
    return NodeDir;
  }

  //---------------------------------------------------
  //Import a new Emit Node
  void copyEmitNode()
  {
    importLua(std::string(PATHTOSRC"/lua_constant/emit.lua"));
  }
  
  // Instead of ccube.lua, we display Centred Cube
  std::string giveNodeName(std::string raw_name) {

	  std::string name = ""; // Name of the node

	  if (raw_name == "ccone.lua") { name = "Centred Cone"; }
	  else if (raw_name == "ccube.lua") { name = "Centred Cube"; }
	  else if (raw_name == "ccylinder.lua") { name = "Centred Cylinder"; }
	  else if (raw_name == "cone.lua") { name = "Cone"; }
	  else if (raw_name == "cube.lua") { name = "Cube"; }
	  else if (raw_name == "cylinder.lua") { name = "Cylinder"; }
	  else if (raw_name == "difference.lua") { name = "Difference"; }
	  else if (raw_name == "emit.lua") { name = "EMIT"; }
	  else if (raw_name == "intersection.lua") { name = "Intersection"; }
	  else if (raw_name == "load.lua") { name = "Load"; }
	  else if (raw_name == "mirror.lua") { name = "Mirror"; }
	  else if (raw_name == "rotate.lua") { name = "Rotate"; }
	  else if (raw_name == "rotate_a.lua") { name = "Rotate (Angular)"; }
	  else if (raw_name == "scale.lua") { name = "Scale"; }
	  else if (raw_name == "sphere.lua") { name = "Sphere"; }
	  else if (raw_name == "translate.lua") { name = "Translate"; }
	  else if (raw_name == "union.lua") { name = "Union"; }
	  else if (raw_name == "vector.lua") { name = "Vector"; }
	  else { name = raw_name; }

	  return name;

  }

  //---------------------------------------------------
  // Create a file tree
  std::string recursiveFileSelecter(std::string current_dir)
  {
	//Create the list of files and directories in the current directorie
    std::vector<std::string> files;
    listLuaFileInDir(files, current_dir);
    std::vector<std::string> directories;
    std::string nameDir = "";
    listFolderinDir(directories, current_dir);
	//Configure the text color for directories
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7, 0.7, 1.0, 1));
	//For all directories create a submenu and call recursively recursiveFileSelecter with the path current_dir/directorie[i]
    ForIndex(i, directories.size()){
      if (ImGui::BeginMenu(directories[i].c_str())){
        nameDir = recursiveFileSelecter(current_dir + "/" + directories[i]);
        ImGui::EndMenu();
      }
    }
	//Delete the old colorstyle of text
    ImGui::PopStyleColor();
	//Set a new style color for files
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1., 1., 1.0, 1));
	//For all files, add a menuItem untitled with file name and set actual nameDir
	
	std::string name = ""; // Name of each node

	if (ImGui::CollapsingHeader("Primitives")) {
		ForIndex(i, files.size()) {
			name = giveNodeName(files[i]);
			if (name == "Centred Cone" || name == "Centred Cube" || name == "Centred Cylinder"
				|| name == "Cone" || name == "Cube" || name == "Cylinder" || name == "Sphere" || name == "Vector") {
				if (ImGui::MenuItem(name.c_str())) {
					nameDir = current_dir + "/" + files[i].c_str();
				}
			}
		}
	}

	if (ImGui::CollapsingHeader("Transformations")) {
		ForIndex(i, files.size()) {
			name = giveNodeName(files[i]);
			if (name == "Mirror" || name == "Rotate" || name == "Rotate (Angular)"
				|| name == "Translate" || name == "Scale") {
				if (ImGui::MenuItem(name.c_str())) {
					nameDir = current_dir + "/" + files[i].c_str();
				}
			}
		}
	}

	if (ImGui::CollapsingHeader("CSG Operations")) {
		ForIndex(i, files.size()) {
			name = giveNodeName(files[i]);
			if (name == "Difference" || name == "Intersection" || name == "Union") {
				if (ImGui::MenuItem(name.c_str())) {
					nameDir = current_dir + "/" + files[i].c_str();
				}
			}
		}
	}

	ForIndex(i, files.size()) {
		name = giveNodeName(files[i]);
		if (!(name == "Centred Cone" || name == "Centred Cube" || name == "Centred Cylinder" || name == "Cone" || name == "Cube" || name == "Cylinder" || name == "Sphere" || 
			name == "Vector" || name == "Difference" || name == "Intersection" || name == "Union" || name == "Mirror" || name == "Rotate" || name == "Rotate (Angular)" || name == "Translate" || name == "Scale")) {
			if (ImGui::MenuItem(name.c_str())) {
				nameDir = current_dir + "/" + files[i].c_str();
			}
		}
	}

    ImGui::PopStyleColor();

    return nameDir;
  }

  //---------------------------------------------------
  // The previous function create a file tree and this one display it
  std::string renderFileSelecter(v2i pos){
    std::string nameDir = "";
    ImGui::Begin("Menu");
    nameDir = recursiveFileSelecter(path + "\\node\\");
    ImGui::SetWindowPos(ImVec2(pos[0], pos[1]));
    ImGui::End();
    return nameDir;

  }

};
