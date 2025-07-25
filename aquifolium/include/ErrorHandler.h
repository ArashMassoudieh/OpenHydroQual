/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@cua.edu for details.
 */


#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <string>
#include <fstream>
#include <vector>
#include <iostream>

#ifdef Q_GUI_SUPPORT
    #include "runtimewindow.h"
    #include "QString"
#endif

using namespace std;

struct _error
{
    string description;
    string cls;
    string funct;
    string objectname;
    int code;
};

class ErrorHandler
{
    public:
        ErrorHandler();
        virtual ~ErrorHandler();
        ErrorHandler(const ErrorHandler& other);
        ErrorHandler& operator=(const ErrorHandler& other);
        void Write(const string &filename);
        bool Append(const _error &err) {errors.push_back(err); return false;}
        int Count() {return errors.size();}
        _error* operator[](int i)
        {
            if (i>=Count())
                return nullptr;
            else
                return &errors[i];
        }
        _error* at(int i)
        {
            if (i>=Count())
                return nullptr;
            else
                return &errors[i];
        }
        void clear() {errors.clear();}
        bool lookup_description(const string &description)
        {
            for (unsigned int i=0; i<errors.size(); i++)
            {
                if (errors[i].description == description)
                    return true;
            }
            return false;
        }
#ifdef Q_GUI_SUPPORT
        void Flush(RunTimeWindow *_rtw=nullptr)
        {
            if (flushed_counter<errors.size())
            {
                for (int j=flushed_counter; j<errors.size(); j++)
                {
                    if (_rtw)
                        _rtw->AppendErrorMessage(QString::fromStdString(errors[j].description));
                    else if (rtw)
                        rtw->AppendErrorMessage(QString::fromStdString(errors[j].description));
                }
            }
            flushed_counter=errors.size();
        }
#endif

        void Flush()
        {
            if (flushed_counter<errors.size())
            {
                for (int j=flushed_counter; j<errors.size(); j++)
                {
                    cout<<"Error: " <<errors[j].description<<std::endl;
                }
            }
            flushed_counter=errors.size();
        }
        bool Append(const string &objectname, const string &cls, const string &funct, const string &description, const int &code)
        {
            if (!lookup_description(description))
            {   _error err;
                err.description = description;
                err.cls = cls;
                err.funct = funct;
                err.objectname = objectname;
                err.code = code;
                errors.push_back(err);
                return false;
            }
            else return false;
        }
#ifdef Q_GUI_SUPPORT
        void SetRunTimeWindow(RunTimeWindow *_rtw) {rtw = _rtw;}
#endif

    protected:

    private:
        int flushed_counter=0;
        vector<_error> errors;
        #ifdef Q_GUI_SUPPORT
        RunTimeWindow *rtw = nullptr;
        #endif // Q_version

};

#endif // ERRORHANDLER_H
