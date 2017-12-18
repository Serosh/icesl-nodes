#include "remote_handler.h"

#include <LibSL/LibSL.h>

#include <string>
#include <map>
#include <vector>
#include <sstream>

#include "project.h"
#include "NodeWindow.h"
#include "GraphSaver.h"
#include "GenericWindow.h"
#include "graphError.h"
#include "GraphSaver.h"

/**
 * This class represents the window of the interface, it contains :
 * -the graph
 * -a console
 * -the project file
 * -the informations on the dimensions of the window and the mouse position
 * -a node selecter which is used when the mous button is pressed
 *
 * WARNING : It cannot be instanciate and the function getInstance() must be used to get a GraphMaker
 */
class GraphMaker{

private:


  NodeGraph m_NodeGraph;
  ConsoleWindow console;
  Project project;//handle copy and paste in the project directory
  bool activeProject; 
  bool m_showRMenu;
  int m_W;		//width of the maker
  int m_H;		//height of the maker
  v2i  m_mousePos;	//position of the mouse in the maker
  NodeSelecter m_nodeSelecter;

  //////private constructor//////////
  GraphMaker(){//private constructor
      activeProject = false; //at the beginning, no project is opened
      m_showRMenu = false;
      m_mousePos = v2i(0,0);
  };


public:
  //list of the Windows in the graph of the project
  std::vector<NodeWindow*>& getNodeWindows(){ return m_NodeGraph.nodeWindows; }

  //function used as a constructor
  static GraphMaker& getInstance( ){
    static GraphMaker n; //instanciation of a graphmaker, the constructor is called, there is at most one GraphMaker 
    return n;
  }

  //function that writes the changes of the graphmaker in the master.lua file
  void onChange();

  //creates a new node (add it in m_NodeGraph.nodeWindows)
  void makeNewNode(std::string path, v2i pos);

  void deleteNodeWindow(NodeWindow* nw);

  bool hasActiveProject(){
      return activeProject;
  }

  //called when the new project button is clicked
  void createProject(std::string path)
  {
       project.path = path; 
       activeProject = true;
       project.createNodefolder(); //create a folder in which the nodes of the project are going to be
       std::string source(PATHTOSRC"/basic_nodes/"); 
       std::string dest(project.nodefolder());
	   puts(source.c_str());
	   puts(project.nodefolder().c_str());
       project.copyDir(source,dest);//function that returns false
       //project.copyEmitNode();
  }
  Project& getProject(){
      return project;
  }

  ///Functions to acces and modify some variables of GraphMaker
  void setShowRMenu(bool b){
      m_showRMenu = b;
  }
  bool isRMenuShown(){
      return m_showRMenu;
  }

  void setMousePos(v2i vec){
      m_mousePos = vec;
  }
  void setWindow(int W, int H){
      m_W = W;
      m_H = H;
      console.setPos(v2i(0,3*m_H/4));
      console.setSize(v2i(m_W,m_H/4));
  }
  ///==========



  void saveGraph(std::string path);
  void loadGraph(std::string path);

  //function that writes errors in the console
  void checkError(script_error& err);

  //function that calls checkerror (modify it to handle new error type)
  void convert_error(script_error& err);

  void nameWindow(NodeWindow* n);

  //updates the nodes 
  void onIdle();

  void renderImgui();

  void RenderMenu();

};

