#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>
#include "Expression.h"
#include <map>

using namespace std;


class System;
class Script;
class Object; 


class Command
{
    public:
        Command(Script *parnt = nullptr);
        virtual ~Command();
        Command(const Command& other);
        Command& operator=(const Command& other);
        Command(const string &s, Script *parnt = nullptr);
        bool Syntax() {return validated;}
        System *GetSystem();
        string LastError() {return last_error;}
        bool Execute(System *sys=nullptr);
        bool Validate(System *sys=nullptr);
        void SetParent (Script *scr);
		vector<Object*> Create2DGrid(System* sys, string name, string type, int n_x, int n_y);
        string Keyword() { return keyword; }
    protected:

    private:
        string keyword;
        vector<string> arguments;
        map<string, string> assignments;
        string last_error;
        bool validated = false;
        Script *parent = nullptr;

};

#endif // COMMAND_H
