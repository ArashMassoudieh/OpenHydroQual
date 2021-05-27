#include "MetaModel.h"
#include <json/json.h>
#ifdef Q_version
    #include <QDebug>
#endif

MetaModel::MetaModel()
{
    //ctor
}

MetaModel::~MetaModel()
{
    //dtor
}

MetaModel::MetaModel(const MetaModel& other)
{
    metamodel = other.metamodel;
}

MetaModel& MetaModel::operator=(const MetaModel& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    metamodel = rhs.metamodel;
    return *this;
}

bool MetaModel::Append(const string &s, const QuanSet &q)
{
    if (metamodel.count(s)>0)
    {
        last_error = "Object type " + s + " Already exist";
        errors.push_back(last_error);
        return false;
    }
    else
        metamodel[s] = q;
    return true;
}

QuanSet* MetaModel::operator[] (const string &typ)
{
    if (metamodel.count(typ)==0)
    {
        last_error = "Type " + typ + " was not found!";
        return nullptr;
    }
    else
        return &(metamodel[typ]);
}

QuanSet* MetaModel::GetItem(const string &typ)
{
    if (metamodel.count(typ)==0)
    {
        last_error = "Type " + typ + " was not found!";
        return nullptr;
    }
    else
        return &(metamodel[typ]);
}

#pragma warning(disable : 4996)
bool MetaModel::GetFromJsonFile(const string &filename)
{
    //qDebug()<<"Clearing!";
    Clear();
    //qDebug()<<"Cleared!";
    Json::Value root;
    Json::Reader reader;

    std::ifstream file(filename);
	if (!file.good())
	{
		cout << "File " + filename + " was not found!";
		return false;
	}
    //else
    //    qDebug()<<"File is good!";

    file >> root;
    //qDebug()<<"File loaded!";
    if(!reader.parse(file, root, true)){
            //for some reason it always fails to parse
        std::cout  << "Failed to parse configuration\n"
                   << reader.getFormattedErrorMessages();
        last_error = "Failed to parse configuration\n";
    }

    //qDebug()<<"Parsed!";


    for (Json::ValueIterator object_types=root.begin(); object_types!=root.end(); ++object_types)
    {
        //qDebug()<<QString::fromStdString(object_types.key().asString());

        if (object_types.key().asString()=="solutionorder")
        {
            for (Json::Value::ArrayIndex i = 0; i != root["solutionorder"].size(); i++)
            {
                solvevariableorder.push_back(root["solutionorder"][i].asString());
            }
            break;
        }
        QuanSet quanset(object_types);
        //qDebug()<<QString::fromStdString(object_types.key().asString());
        Append(object_types.key().asString(),quanset);
    }
	return true;
}

bool MetaModel::AppendFromJsonFile(const string &filename)
{
    Json::Value root;
    Json::Reader reader;

    std::ifstream file(filename);
    if (!file.good())
    {
        cout << "File " + filename + " was not found!";
        return false;
    }
    if (file.good())
    {   Json::CharReaderBuilder b;
        JSONCPP_STRING errs;
        bool ok = Json::parseFromStream(b, file, &root, &errs);
        if (!ok) {
            last_error = errs;
            return false;
        }
    }
    else {
        return false;
    }

    if(!reader.parse(file, root, true)){
            //for some reason it always fails to parse
        std::cout  << "Failed to parse configuration\n"
                   << reader.getFormattedErrorMessages();
        last_error = "Failed to parse configuration\n";
    }


    for (Json::ValueIterator object_types=root.begin(); object_types!=root.end(); ++object_types)
    {
       QuanSet quanset(object_types);
       Append(object_types.key().asString(),quanset);
    }
    return true;
}


void MetaModel::Clear()
{
    solvevariableorder.clear();
    metamodel.clear();
    errors.clear();
}

string MetaModel::ToString(int _tabs)
{
    string out = aquiutils::tabs(_tabs) + "root:\n";
    out += aquiutils::tabs(_tabs) + "{\n";
    for (map<string, QuanSet>::iterator it = metamodel.begin(); it!=metamodel.end(); it++)
        out += metamodel[it->first].ToString(_tabs+1) + "\n";
    out += "}\n";
    return out;
}
