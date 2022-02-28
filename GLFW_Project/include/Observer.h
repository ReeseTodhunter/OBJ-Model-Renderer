#pragma once

#include "Event.h"

//Abstract Observer class for concrete observers to derive from
class Observer
{
public:
	//Constructor & Destructors
	Observer() = default;
	virtual ~Observer() = default;
	//Copy, Move and Assignment operators have been disabled for this class
	Observer(const Observer&) = delete;
	Observer(const Observer&&) = delete;
	Observer& operator = (const Observer&) = delete;

	//Function to perform a call to the abstract call function to post the @param Event to the observer function
	void exec(Event* e)
	{
		call(e);
	}

	//Function to get an Instance of the object/class the function pointer is bound to
	virtual void* Instance() = 0;

private:
	//Abstract call funtion takes an event as a parameter to be implemented in derived classes
	virtual void call(Event* e) = 0;
};

//Template class MemberObserver is a class with a designated observation member function
//Typename T is a pointer to the class owning the function to be called
//Typename ConcreteEvent is the type of event that is being observed
template<typename T, typename ConcreteEvent>
class MemberObserver : public Observer
{
public:
	//typedefine for the pointer to the class member function
	typedef void (T::* MemberFunction)(ConcreteEvent*);
	//Constructor
	MemberObserver(T* a_instance, MemberFunction a_function)
		: m_instance(a_instance), m_memberFunction(a_function)
	{}
	//Destructor
	~MemberObserver() { m_instance = nullptr; }

	//Function to return a pointer to the instance of the class the member function belongs to
	void* Instance() { return(void*)m_instance; }

private:
	//Implementation of abstract base class function for calling the observer function
	void call(Event* e)
	{
		//cast event to correct type
		(m_instance->*m_memberFunction)(static_cast<ConcreteEvent*>(e));
	}

private:
	//The member function in the class that we hold a pointer to
	MemberFunction m_memberFunction;
	//The class instance to call the function to
	T* m_instance;
};

//Template class GlobalObserver, template argument ConcreteEvent is deduced by the compiler
template<typename ConcreteEvent>
class GlobalObserver : public Observer
{
public:
	//typedefine of function to non-member function that takes a concrete event as a parameter
	typedef void (*Function)(ConcreteEvent*);
	//Constructor sets function pointer member to point to parameter function
	GlobalObserver(Function a_function) : m_function(a_function) {}
	//Destructor
	~GlobalObserver() {}
	//Instance function implementation, global functions have no instance so returns nullptr
	void* Instance() { return nullptr; }

private:
	//Call function will call global function member variable with Event parameter
	void call(Event* e)
	{
		//Cast event to correct type
		(*m_function)(static_cast<ConcreteEvent*>(e));;
	}

private:
	//Member variable pointer to global/static function
	Function m_function;
};