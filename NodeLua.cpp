#include "graphMaker.h"
#include "NodeLua.h"
#include <regex>
#include <string>
using namespace std;

int Node::counter = 0;

//-------------------------------------------------------
//open master.lua and read all
void Node::onChange(){
    GraphMaker::getInstance().onChange();
}

//both 2 following functions : change name when new out- or input

//-------------------------------------------------------
void Node::makeNewInput(string name){
    if(getPrevNamed().find(name) != getPrevNamed().end())return;
    getPrevNamed().insert(std::make_pair(name,make_pair(nullptr,""))); 
	// add the name of the node to the map with value the tuple (nullptr,"")
    getInputName().push_back(name); // add the name of the node added to the name of input nodes
}

//-------------------------------------------------------
void Node::makeNewOutput(string name){

    if(getNextNamed().find(name) != getNextNamed().end())return;
    getNextNamed()[name] = nullptr;
    getoutputName().push_back(name); // add the name of the node added to the name of output nodes
    return;  
}

//-------------------------------------------------------
// remove connection in a name when a connection is removed on the window 
void Node::removeConnectionTo(Node* n)
{
    for(auto edge: prevNamed){
        if(edge.second.first == n){
            prevNamed[edge.first] = make_pair(nullptr,"");
        }
    }
    for(auto edge: nextNamed){
        if(edge.second == n){
            nextNamed[edge.first] = nullptr;
        }
    }
}

//-------------------------------------------------------
std::string Node::codeBefore(){
    //write the current Id of the node

    string s = string("setfenv(1, _G0)  --go back to global initialization\n");
    s += string("__curentNodeId = ");
    s += to_string((int64_t)this);
    s += "\n";
    for(auto edge:this->prevNamed){
        string s2 = to_string((int64_t)edge.second.first);
        s+="__inputmap[\""+edge.first+"\"] = \"" + edge.second.second+s2+"\"\n";
    }

    s+= loadFileIntoString(PATHTOSRC"/lua_constant/set_up_env.lua");
    if(hasEmitAndNotOutput && !m_emitingNode){
        s+= loadFileIntoString(PATHTOSRC"/lua_constant/gather_emit.lua");
    }
    for(auto& st: tweaks){
        Tweak* t = st.second;
        s+="__tweaksTable[\""+t->getName()+"\"] = " +  t->getValueToLua() +"\n";
    }
    return s;
}

//-------------------------------------------------------
std::string Node::codeAfter()
{
    string s = "";

    if(hasEmitAndNotOutput && !m_emitingNode){//overwrite emit in node space in lua
        s += loadFileIntoString(PATHTOSRC"/lua_constant/gather_emit_end.lua");
    }
    return s;
}

//------------------------------------------------------------------
void Node::reloadProgram(){
    m_Program = loadFileIntoString(m_Path.c_str());
    m_timeStamp = fileTimestamp(m_Path);
    parse();
    onChange();
}

//-------------------------------------------------------
void Node::parse() {
	std::ifstream input(m_Path.c_str());
	std::string buff;
	std::string buff2;
	getline(input, name);
	getline(input, buff);
	nbOfArgs = stoi(buff);
	for (int i = 0; i < nbOfArgs; i++) {
		getline(input, buff);
		makeNewInput(buff);
	}
	getline(input, buff);
	nbOfTweaks = stoi(buff);
	for (int i = 0; i < nbOfTweaks; i++) {
		getline(input, buff);
		getline(input, buff2);
		if (buff2 == "int") {
			tweaks[buff] = new TweakInt(this, buff, 0);
		}
		else if (buff2 == "string") {
			tweaks[buff] = new TweakString(this, buff, "text");
		}
		else if (buff2 == "float") {
			tweaks[buff] = new TweakScalar(this, buff, 0.0);
		} 
		else if (buff2 == "slider") {
			tweaks[buff] = new TweakSlider(this, buff, 0.0, 0.0, 10.0);
		} 
		else if (buff2 == "path") {
			tweaks[buff] = new TweakPath(this, buff, "C:\\");
		}
	}
	getline(input, lua_template);
	if (m_RelativePath != "emit.lua") {
		makeNewOutput("output");
	}
	input.close();
}


//-------------------------------------------------------
//write the master lua file by traversing the graph.
//bool Node::writeMaster(ofstream& myfile)
//{
//    std::vector<Node*> ordered_node;
//    orderNode(ordered_node);
//    ForIndex(i,ordered_node.size()){
//        Node* currentNode = ordered_node[i];
//        myfile << currentNode->codeBefore().c_str();
//        myfile << currentNode->m_Program.c_str();
//        myfile << currentNode->codeAfter().c_str();
//
//    }
//    return (false); // no error
//}
bool Node::writeMaster(ofstream& myfile) {
	return writeMasterRec(myfile);
}

bool Node::writeMasterRec(ofstream& myfile) {

	for (auto input : inputName) { 
		if (prevNamed[input].first == nullptr) {
			std::cerr << "erreur ecriture" << std::endl;
			return true;
		}//necessaire car si emit sans input et rafraichissement => bug
		prevNamed[input].first->writeMasterRec(myfile);
		
	}
	writeNode(myfile);
	return false;
}

void Node::writeNode(ofstream& myfile) {
	if (!isEmitted) {
		std::string variable = "v" + std::to_string(index);
		std::vector<std::string> args;
		code_to_emit = variable + " = " + lua_template;
		for (std::string& input : inputName) {
			args.push_back("v" + std::to_string(prevNamed[input].first->getIndex()));
		}
		for (std::map<std::string, Tweak*>::iterator it = tweaks.begin(); it != tweaks.end(); it++) {
			std::string to_find = "$" + it->first;
			code_to_emit.replace(code_to_emit.find(to_find), to_find.length(), it->second->getValueOnString());
		}
		int i = 0;
		for (std::string& input : inputName) {
			std::string to_find = "#" + input;
			std::string arg = args[i];
			while (code_to_emit.find(to_find) != std::string::npos) {
				code_to_emit.replace(code_to_emit.find(to_find), to_find.length(), arg);
			}
			i++;
		}
		myfile << code_to_emit << std::endl;
		isEmitted = true;
	}
}

//-------------------------------------------------------
void Node::changePath(std::string path,Project p){
    m_Path = path;
    m_RelativePath = p.relativePath(path);
    m_Program = loadFileIntoString(m_Path.c_str());
    m_emitingNode = strcmp(m_RelativePath.c_str(),"emit.lua") == 0;

    m_timeStamp = fileTimestamp(m_Path);
    parse();
}

//-------------------------------------------------------
std::string Node::hashProgram(){
    hashwrapper* myWrapper = new md5wrapper();
    return myWrapper->getHashFromString(m_Program);
    delete myWrapper;

}

//-------------------------------------------------------
void Node::connect(Node* n,std::string outName,int pos){
    prevNamed.at(inputName[pos]) = std::make_pair(n,outName);
    n->nextNamed[outName] = this;
}

//-------------------------------------------------------
//detect if a node is an Ascendent
bool Node::isAscendent(Node* toConnect){
    std::vector<Node*> toVisit;
    toVisit.push_back(this);
    while(toVisit.size()>0){
        Node* current = toVisit.back();
        toVisit.pop_back();
        if(current == toConnect)return true;
        for( auto& val: current->prevNamed){
            Node* a = val.second.first;
            if(a != nullptr)toVisit.push_back(a);
        }
    }
    return false;
}

//-------------------------------------------------------
//test if all input are connected
bool Node::isConnectedToInput(){
    bool b = true;
    for( auto& val: prevNamed){
        Node* a = val.second.first;
        if(a == nullptr)return false;
        b = b && a->isConnectedToInput();
    }

    return b;
}


//-------------------------------------------------------
//order the node to write the master script.
void Node::orderNode(std::vector<Node*>& orderedNode){
    std::vector<Node*> toVisit;
    toVisit.push_back(this);
    std::set<Node*> visited;
    while(toVisit.size()>0){
        Node* current = toVisit.back();
        if(visited.find(current) == visited.end()){
            orderedNode.push_back(current);
            visited.insert(current);
        }
        toVisit.pop_back();
        for( auto& val: current->prevNamed){
            Node* a = val.second.first;
            if(a == nullptr)continue;
            toVisit.push_back(a);
        }
    }
    std::reverse(orderedNode.begin(),orderedNode.end());
}

//------------------------------------------------------------------
//automatically detect file changes to reload lua files.
Node::t_FileTime Node::fileTimestamp(std::string file) const
{
#ifdef WIN32
  HANDLE f = CreateFileA(file.c_str(),
    GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (f == NULL) {
    t_FileTime z;
    memset(&z, 0, sizeof(t_FileTime));
    return z;
  }
  FILETIME creation, lastaccess, lastwrite;
  GetFileTime(f, &creation, &lastaccess, &lastwrite);
  CloseHandle(f);
  return lastwrite;
#else
  struct stat st;
  int fh = open(file.c_str(), O_RDONLY);
  if (fh < 0) return false;
  fstat(fh, &st);
  close(fh);
  return st.st_mtime;
#endif
}

// -----------------------------------------------
bool Node::fileChanged(std::string file, t_FileTime& _last) const
{
#ifdef WIN32
  HANDLE f = CreateFileA(file.c_str(),
    GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (f == NULL) return false;
  FILETIME creation, lastaccess, lastwrite;
  GetFileTime(f, &creation, &lastaccess, &lastwrite);
  CloseHandle(f);
  if (lastwrite.dwHighDateTime > _last.dwHighDateTime
    || (lastwrite.dwHighDateTime == _last.dwHighDateTime
    &&  lastwrite.dwLowDateTime > _last.dwLowDateTime)
    ) {
    _last = lastwrite;
    cerr << "[script] detected change in " << file << endl;
    return true;
  }
#else
  struct stat st;
  int fh = open(file.c_str(), O_RDONLY);
  if (fh < 0) return false;
  fstat(fh, &st);
  close(fh);
  if (st.st_mtime > _last) {
    _last = st.st_mtime;
    cerr << "[script] detected change in " << file << endl;
    return true;
  }
#endif
  return false;
}


//------------------------------------------------------------------
bool Node::hasChange(){
    return fileChanged(m_Path,m_timeStamp);
}
