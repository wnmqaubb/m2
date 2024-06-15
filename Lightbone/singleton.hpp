<<<<<<< HEAD
#pragma once

template<class T>
class Singleton
{
public:
	Singleton()
	{
	}
	~Singleton()
	{
	}
private:
	Singleton(const Singleton<T>&) = delete;
	Singleton& operator = (const Singleton<T>&) = delete;
public:
	static T &getInstance()
	{
		static T ms_pSingleton;
		return ms_pSingleton;
	}
	static T *instance()
	{
		return &getInstance();
	}
};
=======
#pragma once

template<class T>
class Singleton
{
public:
	Singleton()
	{
	}
	~Singleton()
	{
	}
private:
	Singleton(const Singleton<T>&) = delete;
	Singleton& operator = (const Singleton<T>&) = delete;
public:
	static T &getInstance()
	{
		static T ms_pSingleton;
		return ms_pSingleton;
	}
	static T *instance()
	{
		return &getInstance();
	}
};
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
