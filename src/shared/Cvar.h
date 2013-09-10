/*
===========================================================================

Daemon GPL Source Code
Copyright (C) 2013 Unvanquished Developers

This file is part of the Daemon GPL Source Code (Daemon Source Code).

Daemon Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Daemon Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Daemon Source Code.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/

//#include "../engine/qcommon/q_shared.h"
//#include "../engine/qcommon/qcommon.h"
//#include "Command.h"
#include <string>
#include <functional>

#ifndef SHARED_CVAR_H_
#define SHARED_CVAR_H_

namespace Cvar {

    //TODO more doc
    //TODO more doc (again)

    //All cvars inherit from this class
    class CvarProxy {
        public:
            CvarProxy(std::string name);

            virtual bool OnValueChanged(const std::string& newValue) = 0;

        protected:
            std::string name;

        protected:
            void Register(std::string description, int flags, std::string defaultValue);
            void SetValue(std::string value);
    };

    template<typename T> class Cvar : public CvarProxy{
        public:
            typedef T value_type;

            Cvar(std::string name, std::string description, int flags, std::string defaultValue);
            Cvar(std::string name);

            //Outside code accesses the Cvar value by doing my_cvar.Get() or *my_cvar
            T Get();
            T operator*();

            //Outside code can also change the value
            void Set(T newValue);

            //Called by the cvar system when the value is changed from somewhere else
            virtual bool OnValueChanged(const std::string& text);

        protected:
            bool Parse(std::string text, T& value);
            virtual bool Validate(const T& value);

            T value;
    };

    //TODO do not force people to include functional?
    //Add a callback that watches external changes to a cvar
    template<typename Base> class Callback : public Base {
        public:
            typedef typename Base::value_type value_type;

            template <typename ... Args>
            Callback(std::string name, std::string description, int flags, std::string defaultValue, std::function<void(value_type)> callback, Args ... args);

            template <typename ... Args>
            Callback(std::string name, std::function<void(value_type)> callback, Args ... args);

            virtual bool OnValueChanged(const std::string& newValue);

        private:
            std::function<void(value_type)> callback;
    };

    //Cvars can be extended for different types of values
    bool ParseCvarValue(std::string value, int& result);
    std::string SerializeCvarVale(int value);

    // Cvar<T>

    template<typename T>
    Cvar<T>::Cvar(std::string name, std::string description, int flags, std::string defaultValue): CvarProxy(std::move(name)) {
        Register(std::move(description), flags, std::move(defaultValue));
    }

    template<typename T>
    Cvar<T>::Cvar(std::string name): CvarProxy(std::move(name)) {
    }

    template<typename T>
    T Cvar<T>::Get() {
        return value;
    }

    template<typename T>
    T Cvar<T>::operator*() {
        return this->Get();
    }

    template<typename T>
    void Cvar<T>::Set(T newValue) {
        if (Validate(newValue)) {
            SetValue(SerializeCvarValue(value));
        }
    }

    template<typename T>
    bool Cvar<T>::OnValueChanged(const std::string& text) {
        return Parse(text, value);
    }

    template<typename T>
    bool Cvar<T>::Parse(std::string text, T& value) {
        return ParseCvarValue(std::move(text), value);
    }

    template<typename T>
    bool Cvar<T>::Validate(const T& value) {
        return true;
    }

    // Callback<Base>

    template <typename Base>
    template <typename ... Args>
    Callback<Base>::Callback(std::string name, std::string description, int flags, std::string defaultValue, std::function<void(value_type)> callback, Args ... args): Base(std::move(name), std::forward<Args>(args) ...), callback(callback) {
        this->Register(std::move(description), flags, std::move(defaultValue));
    }

    template <typename Base>
    template <typename ... Args>
    Callback<Base>::Callback(std::string name, std::function<void(value_type)> callback, Args ... args): Base(std::move(name), std::forward<Args>(args) ...), callback(callback) {
    }

    template <typename Base>
    bool Callback<Base>::OnValueChanged(const std::string& newValue) {
        if (Base::OnValueChanged(newValue)) {
            callback(this->Get());
            return true;
        } else {
            return false;
        }
    }
}

#endif // SHARED_CVAR_H_