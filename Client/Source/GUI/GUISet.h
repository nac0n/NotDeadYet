#pragma once
/**
* Author: Rikard Johansson
* Create Date: 28/03/2012
*
* This class will contain a number of buttons and
* other things.
**/

//includes
#include "Graphics.h"
#include "GraphicsEngine.h"

#include "Element.h"
#include "SimpleButton.h"
#include "GUIPicture.h"
#include "TextBox.h"
#include "DropDownList.h"
#include "CheckBox.h"


class GUISet
{
private:
	int mNrOfElements;
	int mMaxNrOfElements;
	Element** mElements;

	GUIEvent* CheckCollision(float mouseX, float mouseY, bool mousePressed, GraphicsEngine* ge);
public:
	GUISet();
	virtual ~GUISet();

	/*! Adds a element to the set*/
	bool AddElement(Element* element);

	/*! Loads all elements into the renderer*/
	bool AddSetToRenderer(GraphicsEngine* ge);

	/*! Removes the set from the renderer*/
	bool RemoveSetFromRenderer(GraphicsEngine* ge);

	/*! Updates the hover and pressed images*/
	GUIEvent* UpdateAndCheckCollision(float mouseX, float mouseY, bool mousePressed, GraphicsEngine* ge);

	/*! Returns the text that is stored in specific text field*/
	string GetTextFromField(string textBox);

	void Resize(float oldWindowWidth, float oldWindowHeight, float windowWidth, float windowHeight);

	/*! Returs the event that is on the top of the Drop down menu*/
	GUIEvent* GetEventFromDropDown(string dropDownMenu);

	CheckBox* GetCheckBox(string checkBox);

	TextBox* GetTextBox(string textBox);
};