#include <LibSL/LibSL.h>
#include <LibSL/LibSL_gl.h>

#include "NodeWindow.h"
#include "NodeLua.h"

#include "imgui/imgui.h" //https://github.com/ocornut/imgui/blob/master/imgui.cpp

using namespace std;

//------------------------------------------------------------------
//render the node
bool NodeWindow::display(){         //Permet l'affichage des fenêtres représentants les noeuds. La fenêtre du menu (pour créer les noeuds "cude", "emit", etc...) s'affiche sans l'appel de cette fonction.

    ImVec2 offsetGUI = ImVec2(m_size[0],m_size[1]);

    ImGui::Begin(m_name.c_str(), &m_show,offsetGUI);    // Push a new ImGui window to add widgets to. Proto : bool ImGui::Begin(const char* nameOfWindow, bool* closeButton, ImGuiWindowFlags flags)
    m_drawList = ImGui::GetWindowDrawList();            // ImDrawList* : qui est la DrawList correspondante à la fenêtre actuelle (issue de GetCurrentWindow())
    handlePosAndSize();
    m_pos = v2i(ImGui::GetWindowPos().x,ImGui::GetWindowPos().y);               // Création de vecteur 2 position de la fenetre
    m_size = v2i(ImGui::GetWindowSize().x,ImGui::GetWindowSize().y);            // Création de vecteur 2 taille de la fenetre
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(cursor);              

    for(auto st: node->getTweaks()){
        st.second->drawUi();
    }
    float err_cont = 5;                    
    ImVec2 minc = ImVec2(ImGui::GetWindowPos().x-err_cont/2,ImGui::GetWindowPos().y-err_cont/2);
    ImVec2 maxc = ImVec2(ImGui::GetWindowPos().x+ImGui::GetWindowSize().x+err_cont/2,ImGui::GetWindowPos().y+ImGui::GetWindowSize().y+err_cont/2);

    //if(ImGui::Button("reload")){
    //    node->reloadProgram();
    //}
    ImGui::End();

    //Node* n = node;
    ForIndex(i,previousConnectedWindow.size()){                         // Pour chaque fenêtre connectée.
        NodeWindow* w= previousConnectedWindow[i];
        if(w == nullptr)continue;
        string s = node->getPrevNamed()[node->getInputName()[i]].second;
        Node* n = w->node;
        int outputSlot = n->getIndiceOutByName(s);
        ImVec2 p1 = w->GetOutputSlotPos(outputSlot);                    // Définition du lien graphique entre les fenêtres.
        ImVec2 p2 = GetInputSlotPos(i);
        ImVec2 p3 = ImVec2(p1.x+50,p1.y);
        ImVec2 p4 = ImVec2(p2.x-50,p2.y);
        m_drawList->AddBezierCurve(p1, p3, p4, p2, ImColor(200,200,100), 3.0f);
    }
    if(node->isInErrorState()){                                         // Gestion d'erreur 
        m_drawList->AddRect(minc,maxc, ImColor(150,0,0,150),10.0,0xFF,err_cont);
    }
    displayNodeName();

    return !m_show;

}

//------------------------------------------------------------------
void NodeWindow::renderAndPick(NodeSelecter &ns, bool mouseDown){       // Créer les cercles qui servent à lier les noeuds entre eux.
    ImColor color = ImColor(150,150,150,150);                           // Gestion de la couleur des cercles en fonction du clic, du drag, de la connection, ...
    v2i Mpos = v2i(ImGui::GetMousePos().x,ImGui::GetMousePos().y);      // Position de la souris

 
    ForIndex(i,node->getInputName().size()){                            // draw input circles
        v2i Cpos = v2i(GetInputSlotPos(i).x,GetInputSlotPos(i).y);
        if(sqLength(Cpos-Mpos) < 100){
            color =ImColor(150,0,0,150);
            if(mouseDown){
                if(!ns.inputHasBeenPicked){
                    ns.nodePickedInput.nodeWindow = this;
                    ns.nodePickedInput.pos = i;
                    ns.inputHasBeenPicked = true;
                }
            }
        }
        if(ns.nodePickedInput.nodeWindow == this && ns.nodePickedInput.pos == i)color =ImColor(0,150,0,150);
        m_drawList->AddCircleFilled(GetInputSlotPos(i), 10, color,64);
        color = ImColor(150,150,150,150);
    }

    ForIndex(i,node->getoutputName().size()){                           // draw output circles
        v2i Cpos = v2i(GetOutputSlotPos(i).x,GetOutputSlotPos(i).y);
        if(sqLength(Cpos-Mpos) < 100){
            color =ImColor(150,0,0,150);
            if(mouseDown){
                if(!ns.outputHasBeenPicked){
                    ns.nodePickedOutput.nodeWindow = this;
                    ns.nodePickedOutput.pos = i;
                    ns.outputHasBeenPicked = true;
                }
            }
        }
        if(ns.nodePickedOutput.nodeWindow == this && ns.nodePickedOutput.pos == i)color =ImColor(0,150,0,150);

        m_drawList->AddCircleFilled(GetOutputSlotPos(i), 10, color,64);
        color = ImColor(150,150,150,150);
    }
    if(ns.nodePickedInput.nodeWindow == this){
        int pos =  ns.nodePickedInput.pos;
        ImVec2 p1 = GetInputSlotPos(pos);
        ImVec2 p2 = ImGui::GetMousePos();
        ImVec2 p3 = ImVec2(p1.x-50,p1.y);
        ImVec2 p4 = ImVec2(p2.x+50,p2.y);
        m_drawList->AddBezierCurve(p1, p3, p4, p2, ImColor(200,200,100), 3.0f);
    }
    if(ns.nodePickedOutput.nodeWindow == this){
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
        //right alignement: displace the cursor by the number of char * the size of a char - the circle ray
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
                //|ImGuiWindowFlags_NoBringToFrontOnFocus
                |ImGuiWindowFlags_AlwaysAutoResize
                );

        float posX= GetInputSlotPos(i).x;// + node->getInputName()[i].size() * char_size;
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
                //|ImGuiWindowFlags_NoBringToFrontOnFocus
                |ImGuiWindowFlags_AlwaysAutoResize
                );
        float posX = GetOutputSlotPos(i).x;
        float posY= GetOutputSlotPos(i).y;
        ImGui::SetWindowPos(ImVec2(posX,posY));
        ImGui::Text(node->getoutputName()[i].c_str());      // Nom du noeud affiché à côté du cercle de lien.
        ImGui::End();

    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();

}

//------------------------------------------------------------------
//connect two nodes. Cycle are not possible
void NodeWindow::connectPreviousWindow(NodeWindow* prev,int inpos,int outpos){ //Permet la connexion entre deux noeuds.
    if(prev->node->isAscendent(node))return;//prevent cycle
    previousConnectedWindow[inpos] = prev;
    Node* n = prev->node;
    node->connect(n,n->getoutputName()[outpos],inpos);
    n->onChange();

}

//------------------------------------------------------------------
//connect two nodes. Cycle are not possible
void NodeWindow::connectPreviousWindow(NodeWindow* prev,string in,string out){

    int outpos = prev->node->getIndiceOutByName(out);
    int inpos = this->node->getIndiceInByName(in);
    connectPreviousWindow(prev,inpos,outpos);
}


