#include "Command.h"
#include "Script.h"
#include <iostream>
#include "System.h"
#include "GA.h"

using namespace std;

Command::Command(Script *parnt)
{
    parent = parnt;
}

Command::~Command()
{
    //dtor
}

Command::Command(const string &s, Script *parnt)
{
    parent = parnt;
    vector<string> firstlevelbreakup = aquiutils::split(s,';');
    {
        if (firstlevelbreakup.size()==0)
        {
            last_error = "Empty line!";
            validated = false;
            return;
        }
    }
    vector<string> maincommand = aquiutils::split(firstlevelbreakup[0],' ');
    if (aquiutils::tolower(maincommand[0])=="loadtemplate" || aquiutils::tolower(maincommand[0])=="addtemplate" || aquiutils::tolower(maincommand[0])=="setasparameter" || aquiutils::tolower(maincommand[0])=="setvalue" || aquiutils::tolower(maincommand[0])=="solve" || aquiutils::tolower(maincommand[0])=="optimize")
    {
        if (maincommand.size()!=1)
            {
                last_error = "Command " + maincommand[0] + " does not require an argument!";
                validated = false;
                return;
            }
        else
        {
            keyword = maincommand[0];
            validated = true;

        }
    }

    if (aquiutils::tolower(maincommand[0])=="echo")
    {
        keyword = maincommand[0];
        validated = true;
    }

    if (aquiutils::tolower(maincommand[0])=="create")
    {
        if (maincommand.size()!=2)
        {
            last_error = "Command 'create' requires one argument! ";
            validated = false;
            return;
        }
        else
        {
            keyword = aquiutils::tolower(maincommand[0]);
            arguments.push_back(maincommand[1]);
            validated = true;
        }
    }

	if (aquiutils::tolower(maincommand[0]) == "write")
	{
		if (maincommand.size() != 2)
		{
			last_error = "Command 'write' requires one argument! ";
			validated = false;
			return;
		}
		else
		{
			keyword = aquiutils::tolower(maincommand[0]);
			arguments.push_back(maincommand[1]);
			validated = true;
		}
	}


    for (unsigned int i=1; i<firstlevelbreakup.size(); i++)
    {
        vector<string> properties = aquiutils::split(firstlevelbreakup[i],',');
        for (unsigned int j=0; j<properties.size(); j++)
        {
            vector<string> prop = aquiutils::split(properties[j],'=');
            if (prop.size()==1)
            {
                last_error = "Property '" + prop[0] + "' does not have a right hand side!";
                assignments[prop[0]] = "";
                //validated = false;
                //break;
            }
            if (prop.size() == 0)
            {
                validated = false;
                return;
            }
            if (prop.size() == 2)
            {
                if (assignments.count(prop[0])==0)
                    assignments[prop[0]] = aquiutils::remove_backslash_r(prop[1]);
                else
                {
                    last_error = "In command '" + s + "': Property '" + prop[0] + "' has been already specified!";
                }
            }
        }
    }
}

Command::Command(const Command& other)
{
    keyword = other.keyword;
    arguments = other.arguments;
    assignments = other.assignments;
    last_error = other.last_error;
    validated = other.validated;
    parent = other.parent;
}

Command& Command::operator=(const Command& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    keyword = rhs.keyword;
    arguments = rhs.arguments;
    assignments = rhs.assignments;
    last_error = rhs.last_error;
    validated = rhs.validated;
    parent = rhs.parent;
    return *this;
}

System *Command::GetSystem()
{
    if (parent)
        return parent->GetSystem();
    else
        return nullptr;

}

bool Command::Execute(System *_sys)
{
    System *sys = nullptr;
    if (_sys==nullptr)
    {
        if (parent->GetSystem()!=nullptr)
        {
            sys = parent->GetSystem();
        }
    }
    else
        sys = _sys;
    if (aquiutils::tolower(keyword) == "loadtemplate")
    {
        if (Validate())
        {
            sys->clear();
            sys->addedtemplates.clear();
            sys->GetMetaModel()->Clear();
						
			if (sys->GetQuanTemplate(assignments["filename"]))
			{
				return true;
			}
            else
            {
                if (!sys->GetQuanTemplate(sys->DefaultTemplatePath() + aquiutils::GetOnlyFileName(assignments["filename"])))
				{
                    //cout<<sys->DefaultTemplatePath() + aquiutils::GetOnlyFileName(assignments["filename"])<<endl;
                    last_error = "File '" + assignments["filename"] + "' was not found!";
					return false;
				}
				else
					return true; 
            }
        }
        else
            return false;
    }
    if (aquiutils::tolower(keyword) == "addtemplate")
    {
        if (Validate())
        {
            if (sys->AppendQuanTemplate(assignments["filename"]))
                return true;
            else
            {
                if (!sys->AppendQuanTemplate(sys->DefaultTemplatePath() + aquiutils::GetOnlyFileName(assignments["filename"])))
				{
					last_error = "File '" + assignments["filename"] + "' was not found!";
					return false;
				}
				else
					return true; 
            }
        }
        else
            return false;
    }

	if (aquiutils::tolower(keyword) == "write")
	{
		if (arguments.size()==0)
		{
			sys->errorhandler.Append("", "Command", "Execute", "In echo command 'writeoutput' an argument is needed", 7023);
			return false;
		}
		if (arguments[0] == "outputs")
		{
			if (assignments.count("filename") == 0)
			{
				sys->errorhandler.Append("", "Command", "Execute", "In echo command 'write' filename must be specified.", 7024);
				return false;
			}
			if (Validate())
			{
				sys->GetOutputs().writetofile(sys->OutputPath() + assignments["filename"]);
			}
		}
		if (arguments[0] == "errors")
		{
			if (assignments.count("filename") == 0)
			{
				sys->errorhandler.Append("", "Command", "Execute", "In echo command 'write' filename must be specified.", 7021);
				return false;
			}
			if (Validate())
			{
				sys->errorhandler.Write(sys->OutputPath() + assignments["filename"]);
			}
		}

    }

    if (aquiutils::tolower(keyword) == "echo")
    {

        if (arguments.size())
        {
            if (arguments[0]=="-")
            {
                cout<<"-----------------------------------------------------------------------------"<<std::endl;
                return true;
            }
            for (unsigned int i=0; i<arguments.size(); i++)
                cout<<arguments[i]<<" ";
            cout<<std::endl;
            return true;
        }
        if (assignments.count("feature")==1 && assignments.count("quantity")==0)
        {
            sys->errorhandler.Append("", "Command", "Execute","In echo command 'quantity' must be specified when feature property is needed.", 7008);
            return false;
        }
        if (Validate())
        {
            if (assignments.count("quantity")==0)
            {
                if (assignments.count("object")==1)
                {
                    return sys->Echo(assignments["object"]);
                }
                else
                {
                    sys->errorhandler.Append("", "Command", "Execute", "In echo command, object must be indicated",7001);
                    return false;
                }
            }
            else
            {
                if (assignments.count("object")==1)
                {
                    if (assignments.count("feature")==1)
                    {
                        return sys->Echo(assignments["object"],assignments["quantity"],assignments["feature"]);
                    }
                    else
                    {
                        return sys->Echo(assignments["object"],assignments["quantity"]);
                    }
                }
                else
                {
                    sys->errorhandler.Append("", "Command", "Execute", "In echo command, object must be indicated",7001);
                    return false;
                }
            }
        }
        else
            return false;
    }

    if (aquiutils::tolower(keyword)=="solve")
    {
        if (Validate())
        {
            sys->SetAllParents();
            if (assignments.count("variable")!=0)
            {
                cout<<"Solving for '" + assignments["variable"] + "'...."<<std::endl;
//              sys->Solve(assignments["variable"],true);
                return true;
            }
            else
            {
                cout<<"Solving for all variables"<<std::endl;
                sys->Solve(true);
                return true;
            }

        }
        else return false;
    }

    if (aquiutils::tolower(keyword)=="optimize")
    {
        if (Validate())
        {
            sys->SetAllParents();
            parent->GetGA()->optimize();
            return true;
        }
        else return false;
    }


    if (aquiutils::tolower(keyword)=="initializeoptimizer")
    {
        if (Validate())
        {
            cout<<"Initializing optimizer...."<<std::endl;
            parent->SetGA(new CGA<System>(sys));
            bool success = true;
            for (map<string,string>::iterator it=assignments.begin(); it!=assignments.end(); it++)
                {
                    if (!parent->GetGA()->SetProperty(it->first,it->second))
                    {
                        sys->errorhandler.Append("", "Command", "Execute", parent->GetGA()->last_error,7021);
                        success = false;
                    };
                }

            return success;
        }
        else return false;
    }

    if (aquiutils::tolower(keyword)=="setvalue")
    {
        if (assignments.count("object")==0)
        {
            sys->errorhandler.Append("", "Command", "Execute", "In the 'setvalue' command, 'object' must be indicated",7010);
            return false;
        }
        if (assignments.count("quantity")==0)
        {
            sys->errorhandler.Append("", "Command", "Execute", "In the 'setvalue' command, 'quantity' must be indicated",7011);
            return false;
        }
        if (assignments.count("value")==0)
        {
            sys->errorhandler.Append("", "Command", "Execute", "In the 'setvalue' command, 'value' must be indicated",7012);
            return false;
        }
        if (aquiutils::tolower(assignments["object"])=="system")
        {
            return sys->SetSystemSettingsObjectProperties(assignments["quantity"],assignments["value"],false);
        }

        if (aquiutils::tolower(assignments["object"])=="optimizer")
        {
            if (parent->GetGA()==nullptr)
            {
                cout<<"Initializing optimizer...."<<std::endl;
                parent->SetGA(new CGA<System>(sys));
            }
            parent->GetGA()->SetProperty(assignments["quantity"],assignments["value"]);
        }

        if (sys->object(assignments["object"])==nullptr && sys->parameter(assignments["object"])==nullptr)
        {
            sys->errorhandler.Append("","Command", "Execute", "Object or parameter '" + assignments["object"] + "' was not found.", 7013);
            return false;
        }
        if (sys->object(assignments["object"])!=nullptr)
        {
            if (!sys->object(assignments["object"])->HasQuantity(assignments["quantity"]))
            {
                sys->errorhandler.Append("","Command","Execute","Object '" + assignments["object"] + "' has no quantity '" + assignments["quantity"] + "'", 7014);
                return false;
            }
            else
            {
                return sys->object(assignments["object"])->Variable(assignments["quantity"])->SetProperty(assignments["value"]);
            }
        }
        else if (sys->parameter(assignments["object"])!=nullptr)
        {
            if (!sys->parameter(assignments["object"])->HasQuantity(assignments["quantity"]))
            {
                sys->errorhandler.Append("","Command","Execute","Parameter '" + assignments["object"] + "' has no quantity '" + assignments["quantity"] + "'", 7014);
                return false;
            }
            else
            {
                return sys->parameter(assignments["object"])->SetProperty(assignments["quantity"],assignments["value"]);
            }
        }
    }

    if (aquiutils::tolower(keyword) == "setasparameter")
    {
        if (Validate())
        {
            sys->SetAsParameter(assignments["object"],assignments["quantity"],assignments["parametername"]);
			if (sys->object(assignments["object"]))
				sys->object(assignments["object"])->Variable(assignments["quantity"])->SetParameterAssignedTo(assignments["parametername"]);
			else
			{
				return false;
				sys->GetErrorHandler()->Append("system", "command", "Execute", "object '" + assignments["object"] + "' was not found!", 11237);
			}
            return true;
        }
        else
            return false;
    }


    if (aquiutils::tolower(keyword)=="create")
    {
        if (sys->GetModel(assignments["type"])==nullptr)
        {
            sys->GetErrorHandler()->Append("system","command","Execute","Type '" + assignments["type"] + "' does not exist!",11234);
            return false;
        }
        if (aquiutils::tolower(arguments[0])=="block")
        {
            if (Validate())
            {
                Block B;

                B.SetName(assignments["name"]);
                B.SetType(assignments["type"]);
                sys->AddBlock(B);
                sys->AddAllConstituentRelateProperties(sys->block(assignments["name"]));
                for (map<string,string>::iterator it=assignments.begin(); it!=assignments.end(); it++)
                {
                    if (it->first!="type" && it->first!="to" && it->first!="from")
                        sys->block(assignments["name"])->SetProperty(it->first,it->second, true, false);
                }
                return true;
            }
            else
                return false;
        }
        if (aquiutils::tolower(arguments[0])=="link")
        {
            if (Validate())
            {
                Link L;
                L.SetName(assignments["name"]);
                L.SetType(assignments["type"]);

				if (sys->AddLink(L, assignments["from"], assignments["to"]))
                {	sys->AddAllConstituentRelateProperties(sys->link(assignments["name"]));
                    L.SetName(assignments["name"]);
					for (map<string, string>::iterator it = assignments.begin(); it != assignments.end(); it++)
					{
						if (it->first != "type" && it->first != "to" && it->first != "from")
                            sys->link(assignments["name"])->SetProperty(it->first, it->second, true, false);
					}
					return true;
				}
				else return false;
            }
            else
                return false;
        }
        if (aquiutils::tolower(arguments[0])=="parameter")
        {
            if (Validate())
            {
                Parameter P;

                sys->AppendParameter(assignments["name"], aquiutils::atof(assignments["low"]), aquiutils::atof(assignments["high"]));
                for (map<string,string>::iterator it=assignments.begin(); it!=assignments.end(); it++)
                {
                    if (it->first!="type" && it->first!="to" && it->first!="from")
                    {
                        if (!sys->parameter(assignments["name"])->SetProperty(it->first,it->second,false))
                            last_error = "Parameter does not have a '" + it->first + "' property!";
                    }
                }
                return true;
            }
            else
                return false;

        }
        if (aquiutils::tolower(arguments[0])=="objectivefunction")
        {
            if (Validate())
            {
                bool succeed = true;
                if (assignments.count("weight")==0)
                    succeed = sys->AppendObjectiveFunction(assignments["name"],assignments["object"],Expression(assignments["expression"]));
                else
                    succeed = sys->AppendObjectiveFunction(assignments["name"],assignments["object"],Expression(assignments["expression"]), aquiutils::atof(assignments["weight"]));
				if (!succeed) return false;
				for (map<string,string>::iterator it=assignments.begin(); it!=assignments.end(); it++)
                {

                    if (!sys->objectivefunction(assignments["name"])->SetProperty(it->first,it->second))
                    {
                        if (it->first != "type")
                        {
                            sys->errorhandler.Append("", "Command", "Execute", "Objective function does not have a '" + it->first + "' + property!", 7021);
                            last_error = "Objective Function does not have a '" + it->first + "' + property!";
                            succeed = false;
                        }
                    }

                }
                return succeed;
            }
            else
                return false;

        }
        if (aquiutils::tolower(arguments[0])=="source")
        {
            if (Validate())
            {
                Source B;
                B.SetName(assignments["name"]);
                B.SetType(assignments["type"]);
                sys->AddSource(B);
                sys->AddAllConstituentRelateProperties(sys->source(assignments["name"]));
                for (map<string,string>::iterator it=assignments.begin(); it!=assignments.end(); it++)
                {
                    if (it->first!="type" && it->first!="to" && it->first!="from")
                        sys->source(assignments["name"])->SetProperty(it->first,it->second,true,false);
                }
                return true;
            }
            else
                return false;
        }
        if (aquiutils::tolower(arguments[0])=="observation")
        {
            if (Validate())
            {
                Observation B;
                B.SetName(assignments["name"]);
                B.SetType(assignments["type"]);
                sys->AddObservation(B);

                for (map<string,string>::iterator it=assignments.begin(); it!=assignments.end(); it++)
                {
                    if (it->first!="type" && it->first!="to" && it->first!="from")
                        sys->observation(assignments["name"])->SetProperty(it->first,it->second);
                }
                return true;
            }
            else
                return false;
        }
        if (aquiutils::tolower(arguments[0])=="constituent")
        {
            if (Validate())
            {
                Constituent B;
                if (B.SetName(assignments["name"]))
                {
                    B.SetType(assignments["type"]);
                    sys->AddConstituent(B);
                    sys->AddConstituentRelateProperties(sys->constituent(assignments["name"]));
                    for (map<string, string>::iterator it = assignments.begin(); it != assignments.end(); it++)
                    {
                        if (it->first != "type")
                        {
                            sys->constituent(assignments["name"])->SetProperty(it->first, it->second, true, false);
                        }
                    }
                    return true;
                }
                else
                    return false; 
            }
            else
                return false;
        }
        if (aquiutils::tolower(arguments[0])=="reaction_parameter")
        {
            if (Validate())
            {
                RxnParameter B;
                B.SetName(assignments["name"]);
                B.SetType(assignments["type"]);
                sys->AddReactionParameter(B);
                for (map<string,string>::iterator it=assignments.begin(); it!=assignments.end(); it++)
                {
                    if (it->first!="type")
                        sys->reactionparameter(assignments["name"])->SetProperty(it->first,it->second,true,false);
                }
                return true;
            }
            else
                return false;
        }
        if (aquiutils::tolower(arguments[0])=="reaction")
        {
            if (Validate())
            {
                Reaction B;
                B.SetName(assignments["name"]);
                B.SetType(assignments["type"]);
                sys->AddReaction(B);
                sys->AddAllConstituentRelateProperties(sys->reaction(assignments["name"]));
                sys->AddConstituentRelateProperties(sys->reaction(assignments["name"]));
                for (map<string,string>::iterator it=assignments.begin(); it!=assignments.end(); it++)
                {
                    if (it->first!="type")
                        sys->reaction(assignments["name"])->SetProperty(it->first,it->second,true,false);
                }
                return true;
            }
            else
                return false;
        }

        return true;
        sys->SetAllParents();
    }
    return false;
}

bool Command::Validate(System *sys)
{
    bool out = true;
    if (parent->MustBeSpecified()->count(keyword)==0)
    {
        last_error = "Keyword '" + keyword + "' is not recognized!";
        if (sys) sys->errorhandler.Append("","Command","Validate",last_error,5001);
        return false;
    }
    if (arguments.size()>0)
        if (parent->MustBeSpecified()->at(keyword).count(arguments[0])==0)
        {
            last_error = "Argument '" + arguments[0] + "' is not recognized for keyword '" + keyword + "'";
            if (sys) sys->errorhandler.Append("","Command","Validate",last_error,5002);
            return false;
        }
    if (arguments.size()>0)
        for (unsigned int i=0; i<parent->MustBeSpecified()->at(keyword)[arguments[0]].size(); i++)
            if (assignments.count(parent->MustBeSpecified()->at(keyword)[arguments[0]][i])==0)
            {
                last_error = "'" + parent->MustBeSpecified()->at(keyword)[arguments[0]][i] + "' must be specified when " + keyword + "ing a " + arguments[0] + "'";
                if (sys) sys->errorhandler.Append("","Command","Validate",last_error,5003);
                return false;
            }

    for (unsigned int i=0; i<parent->MustBeSpecified()->at(keyword)["*"].size(); i++)
        if (assignments.count(parent->MustBeSpecified()->at(keyword)["*"][i])==0)
        {
            last_error = "'" + parent->MustBeSpecified()->at(keyword)["*"][i] + "' must be specified when " + keyword + "ing";
            if (sys) sys->errorhandler.Append("","Command","Validate",last_error,5003);
            return false;
        }

    return out;
}

void Command::SetParent (Script *scr)
{
    parent = scr;
}

vector<Object*> Command::Create2DGrid(System* sys, string name, string type, int n_x, int n_y)
{
	vector<Object*> grid;
	if (!Validate())
		return grid;

	double x0 = atof(assignments["x"].c_str());
	double y0 = atof(assignments["y"].c_str());
	double dx = atof(assignments["dx"].c_str());
	double dy = atof(assignments["dy"].c_str());

	for (int i=0; i<n_x; i++)
		for (int j=0; j<n_y; j++)
		{
			Block B;
			B.SetName(assignments["name"]+"_"+aquiutils::numbertostring(i)+"_"+aquiutils::numbertostring(j));
			B.SetType(assignments["blocktype"]);
			B.SetProperty("x", aquiutils::numbertostring(x0 + i * dx));
			B.SetProperty("y", aquiutils::numbertostring(y0 + i * dy));
			sys->AddBlock(B);
			for (map<string, string>::iterator it = assignments.begin(); it != assignments.end(); it++)
			{
				if (it->first != "name" && it->first != "blocktype" && it->first != "linktype" && it->first != "to" && it->first != "from")
					sys->block(assignments["name"] + "_" + aquiutils::numbertostring(i) + "_" + aquiutils::numbertostring(j))->SetProperty(it->first, it->second);
			}
			grid.push_back(sys->block(assignments["name"] + "_" + aquiutils::numbertostring(i) + "_" + aquiutils::numbertostring(j)));
		}

	for (int i = 0; i < n_x; i++)
		for (int j = 0; j < n_y-1; j++)
		{
			Link L;
			L.SetName(assignments["name"] + "_" + aquiutils::numbertostring(i) + "_" + aquiutils::numbertostring(j) + "-" + assignments["name"] + "_" + aquiutils::numbertostring(i) + "_" + aquiutils::numbertostring(j+1));
			L.SetType(assignments["linktype"]);

			sys->AddLink(L, assignments["name"] + "_" + aquiutils::numbertostring(i) + "_" + aquiutils::numbertostring(j), assignments["name"] + "_" + aquiutils::numbertostring(i) + "_" + aquiutils::numbertostring(j+1));
			for (map<string, string>::iterator it = assignments.begin(); it != assignments.end(); it++)
			{
				if (it->first != "name" && it->first != "blocktype" && it->first != "linktype" && it->first != "to" && it->first != "from")
					sys->block(assignments["name"] + "_" + aquiutils::numbertostring(i) + "_" + aquiutils::numbertostring(j) + "-" + assignments["name"] + "_" + aquiutils::numbertostring(i) + "_" + aquiutils::numbertostring(j + 1))->SetProperty(it->first, it->second);
			}
			grid.push_back(sys->link(assignments["name"] + "_" + aquiutils::numbertostring(i) + "_" + aquiutils::numbertostring(j) + "-" + assignments["name"] + "_" + aquiutils::numbertostring(i) + "_" + aquiutils::numbertostring(j + 1)));
		}
	for (int i = 0; i < n_x-1; i++)
		for (int j = 0; j < n_y; j++)
		{
			Link L;
			L.SetName(assignments["name"] + "_" + aquiutils::numbertostring(i) + "_" + aquiutils::numbertostring(j) + "-" + assignments["name"] + "_" + aquiutils::numbertostring(i+1) + "_" + aquiutils::numbertostring(j));
			L.SetType(assignments["linktype"]);

			sys->AddLink(L, assignments["name"] + "_" + aquiutils::numbertostring(i) + "_" + aquiutils::numbertostring(j), assignments["name"] + "_" + aquiutils::numbertostring(i+1) + "_" + aquiutils::numbertostring(j));
			for (map<string, string>::iterator it = assignments.begin(); it != assignments.end(); it++)
			{
				if (it->first != "name" && it->first != "blocktype" && it->first != "linktype" && it->first != "to" && it->first != "from")
					sys->block(assignments["name"] + "_" + aquiutils::numbertostring(i) + "_" + aquiutils::numbertostring(j) + "-" + assignments["name"] + "_" + aquiutils::numbertostring(i+1) + "_" + aquiutils::numbertostring(j))->SetProperty(it->first, it->second);
			}
			grid.push_back(sys->link(assignments["name"] + "_" + aquiutils::numbertostring(i) + "_" + aquiutils::numbertostring(j) + "-" + assignments["name"] + "_" + aquiutils::numbertostring(i+1) + "_" + aquiutils::numbertostring(j)));
		}
	return grid;
}
