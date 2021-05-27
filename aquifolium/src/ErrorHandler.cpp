#include "ErrorHandler.h"

ErrorHandler::ErrorHandler()
{
    //ctor
}

ErrorHandler::~ErrorHandler()
{
    //dtor
}

ErrorHandler::ErrorHandler(const ErrorHandler& other)
{
    //copy ctor
}

ErrorHandler& ErrorHandler::operator=(const ErrorHandler& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator
    return *this;
}

void ErrorHandler::Write(const string &filename)
{
    ofstream file(filename);
    file << "class, object name, function, description, error code"<<endl;
    for (unsigned int i=0; i<errors.size(); i++)
    {
        file<<errors[i].cls<<","<<errors[i].objectname<<","<<errors[i].funct<<","<<errors[i].description<<","<<errors[i].code<<endl;
    }
    file.close();
}
