#pragma once
#include "Observer.h"
#include "Event.h"

#include <map>
#include <list>
#include <functional>
#include <typeinfo>
#include <typeindex>

//Create a typedefine for std::list<Observer*> objects to improve code readability
typedef std::list<Observer*> ObserverList;

//The dispatcher class handles events and notifies any observers of a particular event occurring
class Dispatcher
{
public:
	static Dispatcher* GetInstance() { return m_instance; }
	static Dispatcher* CreateInstance()
	{
		if(m_instance == nullptr)
		{
			m_instance = new Dispatcher();
		}
		return m_instance;
	}
	static void DestroyInstance()
	{
		if (m_instance)
		{
			delete m_instance;
			m_instance = nullptr;
		}
	}

	//Subscription function subscribes observers to an event, member function pointer implementation
	template<typename T, typename ConcreteEvent>
	void Subscribe(T* a_instance, void(T::*memberFunction)(ConcreteEvent*))
	{
		//Get list of observers from the map
		ObserverList* observers = m_subscribers[typeid(ConcreteEvent)];
		//If observers list is null then there are no observers for this event yet
		if (observers == nullptr)
		{
			//Create a new list for this event type and add it into the subscribers map
			observers = new ObserverList();
			m_subscribers[typeid(ConcreteEvent)] = observers;
		}
		//Push a new observer member onto the observers list
		observers->push_back(new MemberObserver<T, ConcreteEvent>(a_instance, memberFunction));
	}
	//Subscription function for global functions to become event subscribers
	template<typename ConcreteEvent>
	void Subscribe(void(*Function)(ConcreteEvent*))
	{
		//Get a list of observers from the map
		ObserverList* observers = m_subscribers[typeid(ConcreteEvent)];
		//If observers list is null then there are no observers for this event yet
		if (observers == nullptr)
		{
			//Create a new list for this event type and add it into the subscribers map
			observers = new ObserverList();
			m_subscribers[typeid(ConcreteEvent)] = observers;
		}
		//Push a new observer member onto the observers list
		observers->push_back(new GlobalObserver<ConcreteEvent>(Function));
	}
	//Function to publish an event and notify any subscribers
	template<typename ConcreteEvent>
	void Publish(ConcreteEvent* e, bool cleanup = false)
	{
		//Get the list of observers from the map
		ObserverList* observers = m_subscribers[typeid(ConcreteEvent)];
		if (observers == nullptr) { return; }
		for (auto& handler : *observers)
		{
			handler->exec(e);
			//If an event has been handled by a subscriber then we can stop informing any other subscriber as this event is done
			if (static_cast<Event*>(e)->IsHandled())
			{
				break;
			}
		}
		//As this function accepts a pointer to an event the new operator may have been used as an argument
		//We should clean up this allocation if we have been told to
		if (cleanup) { delete e; }
	}

protected:
	//Keep constructors protected and use this dispatcher class as a Singleton object
	Dispatcher() {};
	~Dispatcher()
	{
		//Better clean up the subscriber map
		for (auto it = m_subscribers.begin(); it != m_subscribers.end(); it++)
		{
			ObserverList* obs = it->second;
			for (auto o = obs->begin(); o != obs->end(); o++)
			{
				delete(*o);
				(*o) = nullptr;
			}
			delete obs;
		}
	}

private:
	static Dispatcher* m_instance;
	//A hash map of observers uses the typeId of the event class as an index into the map
	std::map<std::type_index, ObserverList*> m_subscribers;
};