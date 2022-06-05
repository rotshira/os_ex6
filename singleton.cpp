#include <pthread.h>
#include <main1.cpp>
#include <active_object.cpp>
#include <iostream>
#include <pthread.h>
#include <bits/stdc++.h>
using namespace std;
//we took from https://gist.github.com/daniebker/2299755
pthread_mutex_t lock_new = PTHREAD_MUTEX_INITIALIZER;
#ifndef _SINGLETON_H
#define _SINGLETON_H

template<typename T>
class Singleton
{
public:
	static T* GetInstance();
	static void destroy();

private:

	Singleton(Singleton const&){};
	Singleton& operator=(Singleton const&){};

protected:
	static T* m_instance;

	Singleton(){ m_instance = static_cast <T*> (this); };
	~Singleton(){  };
};

template<typename T>
typename T* Singleton<T>::m_instance = 0;

template<typename T>
T* Singleton<T>::GetInstance()
{
    pthread_mutex_lock(&lock_new);
	if(!m_instance)
	{
		Singleton<T>::m_instance = new T();
	}
    pthread_mutex_unlock(&lock_new);
	return m_instance;
}

template<typename T>
void Singleton<T>::destroy()
{
	delete Singleton<T>::m_instance;
	Singleton<T>::m_instance = 0;
}

#endif