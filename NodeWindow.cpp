#include <LibSL/LibSL.h>
#include <LibSL/LibSL_gl.h>

#include "NodeWindow.h"
#include "NodeLua.h"

#include "imgui/imgui.h"//https://github.com/ocornut/imgui/blob/master/imgui.cpp
//imgui is a system enabling developpers to create (among others) graphical interfaces

using namespace std;

//------------------------------------------------------------------
//render the node (original comment)
// Push a new ImGui window to add widgets to. Proto : bool ImGui::Begin(const char* nameOfWindow, bool* closeButton, ImGuiWindowFlags flags)
bool NodeWindow::display(){

    ImVec2 offsetGUI = ImVec2(m_size[0],m_size[1]);

    ImGui::Begin(m_name.c_str(), &m_show,offsetGUI); 
	//ImGui::Begin() makes the click upper right corner to close a window available when 'bool* p_open' is passed as an argument of the function
	// m_name.c_str() is the name of the window, &m_show is the boolean*, offsetGUI are flags
    m_drawList = ImGui::GetWindowDrawList();
	//ImGui::GetWindowDrawList() enables to add custom rendering within a window
	// ImDrawList* : the DrawList corresponding to the current window (from GetCurrentWindow())
    handlePosAndSize();
    m_pos = v2i(ImGui::GetWindowPos().x,ImGui::GetWindowPos().y);               // creation of vector 2 position of the window
	m_size = v2i(ImGui::GetWindowSize().x,ImGui::GetWindowSize().y);            // creation of vector 2 size of the window
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(cursor);              

    for(auto st: node->getTweaks()){
        st.second->drawUi();
    }
    float err_cont = 5;                    
    ImVec2 minc = ImVec2(ImGui::GetWindowPos().x-err_cont/2,ImGui::GetWindowPos().y-err_cont/2);
    ImVec2 maxc = ImVec2(ImGui::GetWindowPos().x+ImGui::GetWindowSize().x+err_cont/2,ImGui::GetWindowPos().y+ImGui::GetWindowSize().y+err_cont/2);

    //if(ImGui::Button("reload")){
    //    node->reloadProgram(); (original comments)
    //}
    ImGui::End(); // finish appending to current window, pop it off the window stack. 
	//(original comment, maybe it means :finish appending to current window, pop the window stack off)

    //Node* n = node; (original comment)
    ForIndex(i,previousConnectedWindow.size()){                         // for each connected window
        NodeWindow* w= previousConnectedWindow[i];
        if(w == nullptr)continue;
        string s = node->getPrevNamed()[node->getInputName()[i]].second;
        Node* n = w->node;
        int outputSlot = n->getIndiceOutByName(s);
        ImVec2 p1 = w->GetOutputSlotPos(outputSlot);                    // definition of the graphical links between windows
        ImVec2 p2 = GetInputSlotPos(i);
        ImVec2 p3 = ImVec2(p1.x+50,p1.y);
        ImVec2 p4 = ImVec2(p2.x-50,p2.y);
        m_drawList->AddBezierCurve(p1, p3, p4, p2, ImColor(200,200,100), 3.0f);
		//m_drawList->AddCircleFilled(p1, 10, ImColor(150, 150, 150, 150), 64);
		//m_drawList->AddCircleFilled(p2, 10, ImColor(150, 150, 150, 150), 64);
		//w->isConnected = true;
    }
    if(node->isInErrorState()){                                         // Gestion d'erreur 
        m_drawList->AddRect(minc,maxc, ImColor(150,0,0,150),10.0,0xFF,err_cont);
    }
    displayNodeName();

    return !m_show;

}

//------------------------------------------------------------------
void NodeWindow::renderAndPick(NodeSelecter &ns, bool mouseDown){       // create circles which link the different nodes to each other
	// NodeSelecter is a structure used to select two nodes to connect them (one is the nodePickedInput, and one is the nodePickedOutput)
	// In fact, nodePickedInput and nodePickedOutput are nodes associated with positions (structure NodePickedInfo)
	ImColor color;
	
	// At the begining, slots are red because diconnected
	color = ImColor(150, 0, 0, 150);                           // Gestion de la couleur des cercles en fonction du clic, du drag, de la connection, ...
    v2i Mpos = v2i(ImGui::GetMousePos().x,ImGui::GetMousePos().y);      // Position de la souris

    //draw input circles (original comment)
    ForIndex(i,node->getInputName().size()){
        v2i Cpos = v2i(GetInputSlotPos(i).x,GetInputSlotPos(i).y); // vector containing the position of the input of the node associated with nodePickedInput

		// Red if on of the input is not connected
		// Grey if all inputs are connected
		if (node->isConnectedToInput2()) {
			color = ImColor(150, 150, 150, 150);
		}
		else {
			color = ImColor(150, 0, 0, 150);
		}

		if(sqLength(Cpos-Mpos) < 100){ // the user has its mouse near to the input of the selected node associated with nodePickedInput
            
			// blue color as red is used for non-connected nodes
			color = ImColor(0,0,150,150);
            
			if(mouseDown){ // the user selects the input of the node associated with nodePickedInput
                if(!ns.inputHasBeenPicked){ // if the selected input node has been picked
                    ns.nodePickedInput.nodeWindow = this;
                    ns.nodePickedInput.pos = i;
                    ns.inputHasBeenPicked = true;
                }
            }
        }
        if(ns.nodePickedInput.nodeWindow == this && ns.nodePickedInput.pos == i)color =ImColor(0,150,0,150);
		// add a circle at the intput of the node associated with nodePickedIntput
		// this node will be connected with the output node found in the next loop
        m_drawList->AddCircleFilled(GetInputSlotPos(i), 10, color,64);
        color = ImColor(150,0,0,150);
    }

    //draw output circles (original comment)
    ForIndex(i,node->getoutputName().size()){ // cycle through (parcourt) the nodes which are connected to the output of the current node            
        v2i Cpos = v2i(GetOutputSlotPos(i).x,GetOutputSlotPos(i).y); // vector containing the position of the output of the node associated with nodePickedOutput

		// Red if the output is not connected
		// Grey if the output is connected
		if (node->isConnectedToOutput()) {
			color = ImColor(150, 150, 150, 150);
		}
		else {
			color = ImColor(150, 0, 0, 150);
		}

		if(sqLength(Cpos-Mpos) < 100){ // if the user has its mouse near to the output of the selected node associated with nodePickedOutput
            
			// changed to blue color as red is used for non-connected outputs
			color = ImColor(0, 0, 150, 150);

            if(mouseDown){ // if the user selects the output of the node associated with nodePickedOutput
                if(!ns.outputHasBeenPicked){ // if the selected output node has been picked
                    ns.nodePickedOutput.nodeWindow = this; 
                    ns.nodePickedOutput.pos = i;
                    ns.outputHasBeenPicked = true;
                }
            }

        }

        if(ns.nodePickedOutput.nodeWindow == this && ns.nodePickedOutput.pos == i)color =ImColor(0,150,0,150); 
		// add a circle at the output of the node associated with nodePickedOutput
		// this node will be connected with the input node found in the previous loop
        m_drawList->AddCircleFilled(GetOutputSlotPos(i), 10, color,64);
        color = ImColor(150,0,0,150);
    }
    if(ns.nodePickedInput.nodeWindow == this){ // draw a curve from the input node to the mouse
        int pos =  ns.nodePickedInput.pos;
        ImVec2 p1 = GetInputSlotPos(pos);
        ImVec2 p2 = ImGui::GetMousePos();
        ImVec2 p3 = ImVec2(p1.x-50,p1.y);
        ImVec2 p4 = ImVec2(p2.x+50,p2.y);
        m_drawList->AddBezierCurve(p1, p3, p4, p2, ImColor(200,200,100), 3.0f);
    }
    if(ns.nodePickedOutput.nodeWindow == this){ // draw a curve from the output node to the mouse
        int pos =  ns.nodePickedInput.pos;
        ImVec2 p1 = GetOutputSlotPos(pos);
        ImVec2 p2 = ImGui::GetMousePos();
        ImVec2 p3 = ImVec2(p1.x+50,p1.y);
        ImVec2 p4 = ImVec2(p2.x-50,p2.y);
        m_drawList->AddBezierCurve(p1, p3, p4, p2, ImColor(200,200,100), 3.0f);
    }


}

// ---------------------------------------------------------
void NodeWindow::displayNodeName()              // Permet l'affichage des noms des noeuds
{
    float MaxInSize = 0;
    float MaxOutSize = 0;
    float char_size = 7.0;
    ForIndex(i,node->getInputName().size()){
        MaxInSize = max(MaxInSize,node->getInputName()[i].size() * char_size);
    }
    ForIndex(i,node->getoutputName().size()){
        //right alignement: displace the cursor by the number of char * the size of a char - the circle ray (original comment)
        MaxOutSize = max(MaxOutSize,node->getoutputName()[i].size() * char_size);
    }
    bool open = true;
    string name = m_name + "around";
    ImGui::PushStyleColor(ImGuiCol_WindowBg,ImVec4(1.0,1.0,1.0,0.0));
    ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.0,0.0,0.0,1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ForIndex(i,node->getInputName().size()){
        ImGui::Begin(name.c_str(),&open,ImVec2(m_size[0],m_size[1]),0.5,
                ImGuiWindowFlags_NoInputs
                |ImGuiWindowFlags_NoMove
                |ImGuiWindowFlags_NoSavedSettings
                |ImGuiWindowFlags_NoTitleBar
                //|ImGuiWindowFlags_NoBringToFrontOnFocus (original comment)
                |ImGuiWindowFlags_AlwaysAutoResize
                );

        float posX= GetInputSlotPos(i).x;// + node->getInputName()[i].size() * char_size; (original comment)
        float posY= GetInputSlotPos(i).y;
        ImGui::SetWindowPos(ImVec2(posX,posY));
        ImGui::Text(node->getInputName()[i].c_str());
        ImGui::End();
    }

    ForIndex(i,node->getoutputName().size()){
        ImGui::Begin(name.c_str(),&open,ImVec2(m_size[0],m_size[1]),0.5,
                ImGuiWindowFlags_NoInputs
                |ImGuiWindowFlags_NoMove
                |ImGuiWindowFlags_NoSavedSettings
                |ImGuiWindowFlags_NoTitleBar
                //|ImGuiWindowFlags_NoBringToFrontOnFocus (original comment)
                |ImGuiWindowFlags_AlwaysAutoResize
                );
        float posX = GetOutputSlotPos(i).x;
        float posY= GetOutputSlotPos(i).y;
        ImGui::SetWindowPos(ImVec2(posX,posY));
        ImGui::Text(node->getoutputName()[i].c_str());      // Nom du noeud affiché a  cote du cercle de lien
        ImGui::End();

    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();

}

//------------------------------------------------------------------
//connect two nodes. Cycles are not possible (original comment)
void NodeWindow::connectPreviousWindow(NodeWindow* prev,int inpos,int outpos){//Permet la connexion entre deux noeuds.
    if(prev->node->isAscendent(node))return;//prevent cycle
    previousConnectedWindow[inpos] = prev;
    Node* n = prev->node;
    node->connect(n,n->getoutputName()[outpos],inpos);
    n->onChange();

}

//------------------------------------------------------------------
//connect two nodes. Cycles are not possible (original comment)
void NodeWindow::connectPreviousWindow(NodeWindow* prev,string in,string out){

    int outpos = prev->node->getIndiceOutByName(out);
    int inpos = this->node->getIndiceInByName(in);
    connectPreviousWindow(prev,inpos,outpos);
}


