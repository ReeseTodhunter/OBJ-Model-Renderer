#pragma once

//Event Class
//An abstract base class for concrete event classes to inherit from

class Event
{
public:
	//Default constructor
	//Constructs a base event class object and sets handled to false.
	Event() : m_bHandled(false) {}
	//Destructor
	virtual ~Event() {};

	//using the 'using' command to create an alias for const char*
	using DescriptorType = const char*;

	//Returns the descriptor type of the event as a const char*
	virtual DescriptorType type() const = 0;

	//Function to check an event has been handled - if not handled event does not report to any subsequent observers
	void Handled() { m_bHandled = true; }

	//Function to set that an Event has been handled
	bool IsHandled() { return m_bHandled; }
private:
	//bool to store if an event has been handled or not
	bool m_bHandled;
};