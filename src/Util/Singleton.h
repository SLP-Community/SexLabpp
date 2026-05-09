#pragma once

template <class T>
class Singleton
{
  protected:
    Singleton() = default;
    ~Singleton() = default;

  public:
    static T* GetSingleton()
    {
        static T singleton;
        return &singleton;
    }

  public:
    Singleton(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;

    Singleton& operator=(const Singleton&) = delete;
    Singleton& operator=(Singleton&&) = delete;
};