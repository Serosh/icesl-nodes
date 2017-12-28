#pragma once
#include <LibSL/LibSL.h>
#include "tweaks.h"
#include "hashlibpp.h"
#include "project.h"
class GraphMaker;

class Node{
private:
private:
#ifdef WIN32
  typedef FILETIME t_FileTime;
#else
  typedef time_t   t_FileTime;
#endif
  t_FileTime m_timeStamp;

protected:
	/* Name of the node's function. Not used at the moment. */
	std::string name;
	/* Number of meshes the node takes as arguments. */
	int nbOfArgs;
	/* Number of tweaks the node takes as arguments. */
	int nbOfTweaks;
	/* Template of the Lua function corresponding to the node. */
	std::string lua_template;
	/* The Lua code that will be written in master.lua. */
	std::string code_to_emit;
	/* True if the corresponding code has already been written in master.lua, else false. */
	bool isEmitted;
	/* Static int that counts the number of nodes. */
	static int counter; 
	/* Unique identifier. 0 for the first node created, 1 for the second, etc. */
	int index;

	/* Stores the inputs of the node.
	 * Key : name of the input.
	 * Value : a pair that couples together a pointer to the input node and the name of the output in the input node. */
	std::map<std::string,std::pair<Node*, std::string>> prevNamed;
	/* Stores the outputs of the node. 
	 * Key : name of the output. 
	 * Value : pointer to the output node. */
	std::map<std::string, Node*> nextNamed;
	/* Stores the names of all the inputs of the nodes. */
	std::vector<std::string> inputName;
	/* Stores the names of all the outputs of the nodes. */
	std::vector<std::string> outputName;
	/* Stores all the tweaks corresponding to the node. 
	 * Key : name of the tweak (ex: "radius").
	 * Value : pointer to the tweak. */
	std::map<std::string, Tweak*> tweaks;

  void makeNewInput(std::string name);
  void makeNewOutput(std::string name);

  std::string m_Path;//absolute path
  std::string m_RelativePath;//path relative to the project directory
  std::string m_Program;//code

  bool m_emitingNode;//tag the special type of node that is emitting into the scene
  bool hasEmitAndNotOutput;//tag the node that are using emit and not output
  bool errorState;//tag the node if it doesn't compile(IceSL give the error back).
public:
  void onChange(); //call back on change on tweaks and others

  void setEmitingNode(bool b){
      m_emitingNode = b;
  }

  void setErrorState(bool b){
      errorState = b;
  }

  bool isInErrorState(){
      return errorState;
  }

  bool isEmitingNode(){
      return m_emitingNode;
  }

  std::map<std::string,Tweak*>& getTweaks(){ return tweaks;}
  Node()
  {
	  isEmitted = false;
      m_emitingNode = false;
      errorState = false;
      hasEmitAndNotOutput = false;

  }
  Node(std::string path, std::string relativePath):m_Path(path)
  {
	isEmitted = false;
    m_RelativePath = extractFileName(relativePath);
    m_Program = loadFileIntoString(m_Path.c_str());
    m_emitingNode = strcmp(m_RelativePath.c_str(),"emit.lua") == 0;
    m_timeStamp = fileTimestamp(m_Path);
    parse();
    errorState = false;
	index = counter;
	counter++;
  }

  int getIndex() {
	  return index;
  }

  int getIndiceOutByName(std::string s){ // go through the list of output nodes of this to find the indice corresponding to the name s 
    ForIndex(i,outputName.size()){
      if(strcmp(s.c_str(),outputName[i].c_str()) == 0){
         return i;
        }
    }
    sl_assert(false);
  }

  int getIndiceInByName(std::string s){// go through the list of input nodes of this to find the indice corresponding to the name s 
    ForIndex(i,inputName.size()){
      if(strcmp(s.c_str(),inputName[i].c_str()) == 0){
         return i;
        }
    }
    sl_assert(false);
  }
  std::map<std::string,std::pair<Node*,std::string>>& getPrevNamed(){return prevNamed;}
  std::map<std::string,Node*>& getNextNamed(){return nextNamed;}
  std::vector<std::string>& getInputName(){return inputName;}
  std::vector<std::string>& getoutputName(){return outputName;  }
  std::string getPath(){return m_Path;}
  void changePath(std::string path,Project p);
  std::string getRelativePath(){return m_RelativePath;}
  void setEmitted(bool isEmitted) { this->isEmitted = isEmitted; }

  std::string codeBefore(); //code to write before writing the node in the master script
  std::string codeAfter(); //code to write after writing the node in the master script
  void removeConnectionTo(Node* n);
  std::string hashProgram(); //hash the file
  void writeNode(std::ofstream& myfile);
  bool writeMasterRec(std::ofstream& myfile);
  bool writeMaster(std::ofstream& myfile); //write the master script of the node.
  void connect(Node* n,std::string outName,int pos); //connect to another node
  bool isAscendent(Node* toConnect); // check if a node is a parent
  bool isConnectedToInput(); //check if all input are conencted
  bool isConnectedToInput2(); // check if inputs of node are connected (non recursive)
  bool isConnectedToOutput(); // check if output is connected
  void orderNode(std::vector<Node*>& orderedNode); //order the nodes before writing master script
  void parse(); //parse to find input, output, gather emit
  void parseTweaks(); //parse to find the tweaks and add them in the interface.
  void parseInputAndOutput();
  void reloadProgram();
  t_FileTime fileTimestamp(std::string file) const;
  bool fileChanged(std::string file, t_FileTime& _last) const;
  bool hasChange();
};

