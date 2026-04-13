#pragma once

//-------------------------------------------------------------------------------------------------

class GUI_ListBoxMouseMoveHover final : public juce::MouseListener, public juce::ChangeBroadcaster
{
public:
	GUI_ListBoxMouseMoveHover ( juce::TableListBox& lb ) : owner ( lb )
	{
		owner.addMouseListener ( this, true );
	}

	~GUI_ListBoxMouseMoveHover () override
	{
		owner.removeMouseListener ( this );
	}

	void mouseEnter ( const juce::MouseEvent& e ) override
	{
		updateHoverPos ( e );
	}

	void mouseMove ( const juce::MouseEvent& e ) override
	{
		updateHoverPos ( e );
	}

	void mouseExit ( const juce::MouseEvent& /*e*/ ) override
	{
		hoverPos = -1;
		sendSynchronousChangeMessage ();
	}

	int getHoverPos () const
	{
		return hoverPos;
	}

private:
	juce::TableListBox& owner;
	int					hoverPos = -1;

	void updateHoverPos ( const juce::MouseEvent& e )
	{
		// Get hovered row
		auto	pos = e.getEventRelativeTo ( &owner ).position.toInt ();
		auto	row = owner.getRowContainingPosition ( pos.x, pos.y );
		auto	rowEnd = owner.getRowPosition ( row, false ).getWidth ();

		if ( pos.x < 0 || pos.x > rowEnd || e.eventComponent == &owner )
			row = -1;

		// Update owner
		if ( row != hoverPos )
		{
			hoverPos = row;
			sendSynchronousChangeMessage ();
		}
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_ListBoxMouseMoveHover )
};
//-------------------------------------------------------------------------------------------------
