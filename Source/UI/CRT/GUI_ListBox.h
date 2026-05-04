#pragma once

#include <JuceHeader.h>

#include "UI/Misc/GUI_ListboxMouseMoveHover.h"

#include "Globals/Icons.h"
#include "Globals/Strings.h"

//-----------------------------------------------------------------------------

struct browserEntry
{
	int				type;	// 0 = CRT, 1 = PRG
	bool			official;

	juce::String	name;
	juce::String	path;

	std::string		normalized;
};
//-----------------------------------------------------------------------------

class GUI_ListBox : public juce::TableListBox, public juce::TableListBoxModel, public juce::ChangeListener
{
public:
	GUI_ListBox ();
	~GUI_ListBox () override;

	// juce::ListBox
	bool keyPressed ( const juce::KeyPress& key ) override;

	// juce::TableListBox
	void paint ( juce::Graphics& g ) override;

	// juce::TableListBox
	void resized () override;
	void paintRowBackground ( juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected ) override;
	void paintCell ( juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected ) override;

	// juce::TableListBoxModel
	void returnKeyPressed ( int rowNumber ) override;
	void cellDoubleClicked ( int rowNumber, int columnId, const juce::MouseEvent& e ) override;
	juce::String getCellTooltip ( int rowNumber, int columnId ) override;

	int getNumRows () override;
	void sortOrderChanged ( int newSortColumnId, bool isForwards ) override;

	juce::var getDragSourceDescription ( const juce::SparseSet<int>& selectedRows ) override;

	// this
	void addHeaderColumn ( const int colId, bool sortable = false );
	void setRowData ( std::vector<browserEntry*>& newData );

	// juce::ChangeListener
	void changeListenerCallback ( juce::ChangeBroadcaster* source ) override;
	int		hoverPosition = -1;
	GUI_ListBoxMouseMoveHover	hover;

protected:
	std::vector<browserEntry*>	rowData;

	juce::SharedResourcePointer<Icons>		icons;
	juce::SharedResourcePointer<Strings>	strings;
};
//-----------------------------------------------------------------------------
