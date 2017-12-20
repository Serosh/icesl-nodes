#pragma once

#include <LibSL/LibSL.h>

#include "imgui/imgui.h"
#include "NodeLua.h"


/************************************/
/************************************/
/************************************/


class GenericWindow {
protected:
	bool m_show;
	//m_size = v2i[0] * v2i[1]
	v2i m_size;
	v2i m_pos;
	//On indique si on travaille sur une ancienne ou nouvelle fenêtre avec m_isNew
	bool m_isNew;
	//Nom de la fenêtre
	std::string m_name;
	//Listes de commandes à effectuer (ex AddLine, AddRect...)
	ImDrawList* m_drawList;
public:
	//---------------------------------------------------------
	//Constructeurs

	GenericWindow() :m_size(v2i(100, 100)), m_pos(v2i(100, 100))
	{
		m_show = true;
		m_isNew = true;
	}

	GenericWindow(const char *name, v2i pos) :m_show(true), m_size(v2i(100, 100)), m_pos(pos), m_name(name)
	{
		m_show = true;
		m_isNew = true;
	}
	GenericWindow(const char *name, v2i pos, v2i size) :m_show(true), m_size(size), m_pos(pos), m_name(name) {

	}

	GenericWindow(const char *name) :m_show(true), m_size(v2i(100, 100)), m_pos(v2i(100, 100)), m_name(name)
	{
		m_show = true;
		m_isNew = true;
	}

	//Fin Constructeurs 
	//---------------------------------------------------------
	//Setters & getters

	v2i getSize() { return m_size; }
	v2i getPos() { return m_pos; }
	void setSize(v2i s) { m_size = s; m_isNew = true; }
	void setPos(v2i p) { m_pos = p;  m_isNew = true; }

	//Fin Setters & getters
	//---------------------------------------------------------

	void handlePosAndSize() {	//Actualise la position et taille de la nouvelle fenêtre, et décrête qu'elle n'est plus nouvelle
		if (m_isNew) {
			ImVec2 posVec2 = ImVec2(m_pos[0], m_pos[1]);
			ImVec2 sizeVec2 = ImVec2(m_size[0], m_size[1]);
			ImGui::SetWindowPos(posVec2);
			ImGui::SetWindowSize(sizeVec2);
			m_isNew = false;
		}
	}

	//---------------------------------------------------------
	//Setter et getter

	std::string getName() { return m_name; }
	void setName(std::string str) { m_name = str; }

	//Fin setter et getter
	//---------------------------------------------------------

	//C'est la méthode qui permet d'afficher la fenêtre, mais elle est propre à chaque classe héritée et donc on la met virtuelle.
	virtual bool display() = 0;
};


//write the error message comming back from IceSL
//at the bottom of the main window

class ConsoleWindow :public GenericWindow {
	//Message de console à afficher
	std::string m_message;
	//Si on saute une ligne, m_scrollToBottom prend la valeur true
	bool m_scrollToBottom;
public:

	//---------------------------------------------------------
	//Constructeurs

	ConsoleWindow() :GenericWindow() { m_message = std::string(); }
	ConsoleWindow(v2i pos, v2i size) :GenericWindow("console", pos, size), m_message("") {}

	//Fin constructeurs
	//---------------------------------------------------------
	//Ajout d'un message à la console

	void append(std::string str) {
		m_message += "\n";
		m_message += str;
		m_scrollToBottom = true;
	}

	//---------------------------------------------------------
	//Nettoyer la console

	void clear() {
		m_message = "";
		m_scrollToBottom = true;
	}

	//---------------------------------------------------------
	//Implémentation de la méthode d'affichage de la fenêtre

	bool display() {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		if (strcmp(m_message.c_str(), "") == 0) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7, 1.0, 0.7, 1));
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 0.0, 0.0, 1));

		}
		//Ajoute la fenêtre à la stack et on peut commencer à ajouter des éléments dessus.
		ImGui::Begin(m_name.c_str(), &m_show, ImVec2(m_size[0], m_size[1]), -1.0f, ImGuiWindowFlags_NoInputs |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoTitleBar
		);
		//Initialise la fenêtre en taille et position
		handlePosAndSize();

		//Wrap du texte à la fin de la fenêtre
		ImGui::TextWrapped(m_message.c_str());
		//Juste crée un séparateur en dessous du texte.
		ImGui::Separator();
		if (m_scrollToBottom) {	//Si on a sauté une ligne
			ImGui::SetScrollHere();		//On peut scroller jusque cette ligne
		}
		m_scrollToBottom = false;	//On réinitialise ce bool.
		ImGui::End();	//on finit d'ajouter des éléments à la fenêtre en sortant la fenêtre de la stack


		ImGui::PopStyleVar();
		ImGui::PopStyleColor();

		return true;	//la fenêtre s'est correctement construite
	}
};
