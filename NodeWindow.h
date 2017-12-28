#pragma once

#include <LibSL/LibSL.h>

#include "imgui/imgui.h"
#include "NodeLua.h"
#include "GenericWindow.h"


#ifndef WIN32
#include <sys/stat.h>
#include <fcntl.h>
#endif

/************************************/
/************************************/
/************************************/


struct NodeSelecter;


class NodeWindow:public GenericWindow{
    Node* node;
    std::vector<NodeWindow*> previousConnectedWindow; // contains all the windows which are connected to the window of the Node node (to its input)
	//bool isConnected = false;

public:

    NodeWindow():GenericWindow(){}
    NodeWindow(Node*n, const char *name,v2i pos):GenericWindow(name,pos),node(n)
    {
        previousConnectedWindow.resize(n->getInputName().size()); 
		// resize the vector previousConnectedWindow 
	}

    NodeWindow(Node*n, const char *name):GenericWindow(name),node(n)
    {
        m_show = true;
        previousConnectedWindow.resize(n->getInputName().size()); 
		// resize the vector previousConnectedWindow 
    }

    void drawInputValue();
    void drawOutputValue();

    void connectPreviousWindow(NodeWindow* prev, int inpos, int outpos); // connect the window to its previous nodes
    void connectPreviousWindow(NodeWindow* prev,std::string in, std::string out); // connect the window to its previous nodes
    void nodeChange(){
        previousConnectedWindow.resize(node->getInputName().size());
		// resize the vector previousConnectedWindow 
    }

    Node* getNode(){return node;}
    void setNode(Node* n){
        node =n;
        previousConnectedWindow.resize(n->getInputName().size());
    }
    ImVec2 GetInputSlotPos(int slot_no)   const { return ImVec2(m_pos[0], m_pos[1] + m_size[1] * ((float)slot_no+1) / (node->getPrevNamed().size()+1.0)); }
    ImVec2 GetOutputSlotPos(int slot_no)  const { return ImVec2(m_pos[0] + m_size[0], m_pos[1] + m_size[1] * ((float)slot_no+1) / (node->getNextNamed().size()+1.0)); }

	
    bool display();// display the window of the node
    void renderAndPick(NodeSelecter &ns, bool mouseDown, bool mouseDownMiddle);
    void displayNodeName();

	/* the node nw is removed from the vector of nodes connected to "this" */
    void removeConnectionTo(NodeWindow* nw){
        ForIndex(i,previousConnectedWindow.size()){
            if(previousConnectedWindow[i] == nw){
                previousConnectedWindow[i] = nullptr;
            }
        }
		
    }
};
//structure containing a node and the position of this node (I think)
struct NodePickedInfo{
    NodeWindow* nodeWindow;
    int pos;

    NodePickedInfo(){
        nodeWindow = nullptr;
        pos = 0;
    }

    NodePickedInfo(NodeWindow* nw, int pos){
        nodeWindow = nw;
        pos = pos;
    }
} ;


//used to select two nodes to connect them or to remove the connection between them
struct NodeSelecter{

    NodePickedInfo nodePickedInput;
    NodePickedInfo nodePickedOutput;
	NodePickedInfo nodePickedInputToDelete;
	NodePickedInfo nodePickedOutputToDelete;

    bool inputHasBeenPicked;
    bool outputHasBeenPicked;
	bool inputHasBeenPickedToDelete;
	bool outputHasBeenPickedToDelete;

    NodeSelecter(){
        inputHasBeenPicked  = false;
        outputHasBeenPicked = false;
		inputHasBeenPickedToDelete = false;
		outputHasBeenPickedToDelete = false;
    }

	// If nothing has been picked but mouse was down, we make a reset
    void reset(){
        nodePickedInput.nodeWindow  = nullptr;
        nodePickedOutput.nodeWindow = nullptr;
        nodePickedInput.pos  = 0;
        nodePickedOutput.pos = 0;
		nodePickedInputToDelete.nodeWindow = nullptr;
		nodePickedOutputToDelete.nodeWindow = nullptr;
		nodePickedInputToDelete.pos = 0;
		nodePickedOutputToDelete.pos = 0;
    }

    void connect(){
        if(nodePickedInput.nodeWindow == nullptr)return;
        if(nodePickedOutput.nodeWindow == nullptr)return;
        if(nodePickedInput.pos < 0)return;
        if(nodePickedOutput.pos < 0)return;
        
		nodePickedInput.nodeWindow->connectPreviousWindow(nodePickedOutput.nodeWindow,nodePickedInput.pos,nodePickedOutput.pos);
		// add the output node to the list of nodes connected to the input node 
        reset();
    }

	void disconnect() {
		if (nodePickedInputToDelete.nodeWindow == nullptr)return;
		if (nodePickedOutputToDelete.nodeWindow == nullptr)return;
		if (nodePickedInputToDelete.pos < 0)return;
		if (nodePickedOutputToDelete.pos < 0)return;

		// Remove connections both between Nodes and NodeWindows
		nodePickedInputToDelete.nodeWindow->removeConnectionTo(nodePickedOutputToDelete.nodeWindow);
		nodePickedOutputToDelete.nodeWindow->removeConnectionTo(nodePickedInputToDelete.nodeWindow);
		nodePickedInputToDelete.nodeWindow->getNode()->removeConnectionTo(nodePickedOutputToDelete.nodeWindow->getNode());
		nodePickedOutputToDelete.nodeWindow->getNode()->removeConnectionTo(nodePickedInputToDelete.nodeWindow->getNode());
		
		reset();
	}

};
