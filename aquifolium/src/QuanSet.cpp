#include "QuanSet.h"
#include "Object.h"
#include "System.h"
#include <json/json.h>
#include <list>

#ifdef Q_version
    #include <QDebug>
#endif

#ifdef QT_version
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#endif // Qt_version


QuanSet::QuanSet()
{
    parent = nullptr;
}

QuanSet::~QuanSet()
{
    quans.clear(); 
    quantity_order.clear(); 
}

QuanSet::QuanSet(Json::ValueIterator& object_types)
{
    parent = nullptr;
    Name() = object_types.key().asString();
    //qDebug()<<"Name: " << QString::fromStdString(Name());
    ObjectType = "Entity";
	BlockLink = blocklink::entity;
	for (Json::ValueIterator it=object_types->begin(); it!=object_types->end(); ++it)
    {
        if (it.key()=="icon")
            IconFileName() = (*it)["filename"].asString();
		else if (it.key()=="typecategory")
			typecategory = (*object_types)[it.key().asString()].asString();
        else if (it.key()=="normalizing_quantity")
        {
            normalizing_quantity = (*object_types)[it.key().asString()].asString();
        }
        else if (it.key()=="type")
        {
			string _type = (*object_types)[it.key().asString()].asString();
			if (_type == "block")
			{
				BlockLink = blocklink::block;
				ObjectType = "Block";
			}
			else if (_type == "link")
			{
				BlockLink = blocklink::link;
				ObjectType = "Connector";
			}
			else if (_type == "source")
			{
				BlockLink = blocklink::source;
				ObjectType = "Source";
			}
            else if (_type == "Reaction")
            {
                BlockLink = blocklink::reaction;
                ObjectType = "Reaction";
            }
            else
			{
				BlockLink = blocklink::entity;
				ObjectType = "Entity";
			}
        }
		else if (it.key()=="description")
        {
            description = (*object_types)[it.key().asString()].asString();
        }
        else
        {
            if (it->size()!=0)
            {
                //qDebug()<<QString::fromStdString(it.key().asString());
                //qDebug()<<QString::fromStdString(it.key().asString());
                Quan Q(it);
                //qDebug()<<"Appending" << QString::fromStdString(it.key().asString());
                Append(it.key().asString(),Q);
                //qDebug()<<"Appended";
                //qDebug()<<QString::fromStdString(Q.ToString());
            }
            else {
                AppendError(it.key().asString(),"QuanSet","Constructor","Syntax error in '" + it.key().asString(),18021);
            }
        }
    }
    if (this->quans.count("name")==0)
    {
        Quan Q;
        Q.SetName("name");
        Q.SetType(Quan::_type::string);
        Q.Description() = "Name";
        Q.Delegate() = "String";
        Q.AskFromUser() = true;
        Append("name",Q);
    }
    if (ObjectType == "Block")
    {
        if (this->quans.count("x")==0)
        {
            Quan Q;
            Q.SetName("x");
            Q.SetType(Quan::_type::value);
            Q.Description() = "x";
            Q.Default() = "900";
            Q.AskFromUser() = true;
            Q.Delegate() = "ValueBox";
            Append("x",Q);
        }
        if (this->quans.count("y")==0)
        {
            Quan Q;
            Q.SetName("y");
            Q.SetType(Quan::_type::value);
            Q.Description() = "y";
            Q.Default() = "900";
            Q.AskFromUser() = true;
            Q.Delegate() = "ValueBox";
            Append("y",Q);
        }
        if (this->quans.count("_width")==0)
        {
            Quan Q;
            Q.SetName("_width");
            Q.SetType(Quan::_type::value);
            Q.Description() = "_width";
            Q.Default() = "200";
            Q.AskFromUser() = true;
            Q.Delegate() = "ValueBox";
            Append("_width",Q);
        }
        if (this->quans.count("_height")==0)
        {
            Quan Q;
            Q.SetName("_height");
            Q.SetType(Quan::_type::value);
            Q.Description() = "_height";
            Q.Default() = "200";
            Q.AskFromUser() = true;
            Q.Delegate() = "ValueBox";
            Append("_height",Q);
        }

    }
    //qDebug()<<QString::fromStdString(Name())<< " Done!";
}

QuanSet::QuanSet(const QuanSet& other)
{
    quans = other.quans;
    iconfilename = other.iconfilename;
    description = other.description;
    BlockLink = other.BlockLink;
	ObjectType = other.ObjectType;
    name = other.name;
	typecategory = other.typecategory;
    quantity_order = other.quantity_order;
    normalizing_quantity = other.normalizing_quantity;
    parent = nullptr;


}

QuanSet& QuanSet::operator=(const QuanSet& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    name = rhs.name;
    quans = rhs.quans;
    iconfilename = rhs.iconfilename;
    description = rhs.description;
    BlockLink = rhs.BlockLink;
	ObjectType = rhs.ObjectType;
	typecategory = rhs.typecategory;
    quantity_order = rhs.quantity_order;
    normalizing_quantity = rhs.normalizing_quantity;
    parent = nullptr;
    return *this;
}

bool QuanSet::Append(const string &s, const Quan &q)
{
    if (Find(s))
    {
        AppendError(Name(),"QuanSet","Append","In " + Name() + ": Quantity " + s + " Already exists!",2001);
        return false;
    }
    else
    {
        //qDebug()<<QString::fromStdString(q.ToString());
        quans[s] = q;
        quans[s].SetParent(this->parent);
        //qDebug()<<"Done!";
        quantity_order.push_back(s);
        //qDebug()<<"Done!";
        return true;
    }

}

void QuanSet::Append(QuanSet &qset)
{

    for (unordered_map<string, Quan>::iterator it = qset.begin(); it!=qset.end(); it++)
    {
    #ifdef Debug_mode
    ShowMessage(it->second.GetName() + "  " + qset[it->first].GetName() + "  " + it->first);
    #endif // Debug_mode
        Append(it->second.GetName(),qset[it->first]);
    }

}

Quan& QuanSet::operator[] (const string &s)
{
    if (!Find((s)))
    {
        last_error = "Variable " + s + " does not exist!";
    }
    else
        return quans.at(s);
}

Quan& QuanSet::GetVar(const string &s)
{
    if (!Find(s))
    {
        AppendError(Name(),"QuanSet","GetVar","Variable " + s + " does not exist!",2001);
    }
    else
        return quans.at(s);
}

Quan* QuanSet::GetVar(int i)
{
    if (int(quans.size())<=i)
    {
        AppendError(Name(),"QuanSet","GetVar","Variable " + aquiutils::numbertostring(i) + " does not exist!",2001);
        return nullptr;
    }
    else
    {
        int j=0;
        for (unordered_map<string,Quan>::iterator it=quans.begin(); it!=quans.end(); it++)
        {
            if (j==i)
                return &it->second;
            j++;
        }
        return nullptr;
    }
}

void QuanSet::UnUpdateAllValues()
{
    for (unordered_map<string, Quan>::iterator it = quans.begin(); it != quans.end(); it++)
        it->second.Set_Value_Update(false);
}

Quan* QuanSet::GetVarAskable(int i)
{
    int j=0;
    for (unordered_map<string,Quan>::iterator it=quans.begin(); it!=quans.end(); it++)
    {
        if (j==i && it->second.AskFromUser() && !it->second.WhenCopied())
           return &it->second;


        if (it->second.AskFromUser() && !it->second.WhenCopied()) j++;

    }

    AppendError(Name(),"QuanSet","GetVar","Variable " + aquiutils::numbertostring(i) + " does not exist or is not askable!",2001);
    return nullptr;

}

unsigned long QuanSet::AskableSize()
{
    unsigned int j=0;
    for (unordered_map<string,Quan>::iterator it=quans.begin(); it!=quans.end(); it++)
    {
        if (it->second.AskFromUser() && !it->second.WhenCopied() ) j++;
    }
    return j;
}

string QuanSet::ToString(int _tabs)
{
    string out = aquiutils::tabs(_tabs) + name + ":\n";
    out += aquiutils::tabs(_tabs) + "{\n";
    if (BlockLink == blocklink::block)
        out += aquiutils::tabs(_tabs+1) + "type: block\n";
    else if (BlockLink == blocklink::link)
        out += aquiutils::tabs(_tabs+1) + "type: link\n";

    if (iconfilename!="")
    {
        out += aquiutils::tabs(_tabs+1) + "icon: {\n";
        out += aquiutils::tabs(_tabs+2) + "filename: " + iconfilename + "\n";
        out += aquiutils::tabs(_tabs+1) + "}\n";
    }


    for (unordered_map<string,Quan>::iterator it=quans.begin(); it!=quans.end(); it++)
    {
        out += quans[it->first].ToString(_tabs+1) + "\n";
    }

    out += aquiutils::tabs(_tabs) + "}\n";
    return out;
}


void QuanSet::ShowMessage(const string &msg)
{
    if (parent)
        if (parent->Parent())
            if (!parent->Parent()->IsSilent()) cout<<msg<<std::endl;
}

void QuanSet::SetAllParents()
{
    for (unordered_map<string,Quan>::iterator it=quans.begin(); it!=quans.end(); it++)
        quans[it->first].SetParent(parent);
}

bool QuanSet::AppendError(const string &objectname, const string &cls, const string &funct, const string &description, const int &code)
{
    if (!parent)
        return false;
    if (!parent->Parent())
        return false;
#pragma omp critical (appenderror)
    parent->Parent()->errorhandler.Append(objectname,cls,funct,description,code);
    return true;
}

SafeVector<CTimeSeries<timeseriesprecision>*> QuanSet::TimeSeries()
{

    SafeVector<CTimeSeries<timeseriesprecision>*> out;
    for (unordered_map<string,Quan>::iterator it=quans.begin(); it!=quans.end(); it++)
    {
        if (quans[it->first].GetType() == Quan::_type::timeseries && quans[it->first].TimeSeries()!=nullptr)
            out.push_back(quans[it->first].TimeSeries());
    }
    for (unordered_map<string, Quan>::iterator it = quans.begin(); it != quans.end(); it++)
    {
        if (quans[it->first].GetType() == Quan::_type::prec_timeseries && quans[it->first].TimeSeries() != nullptr)
            out.push_back(quans[it->first].TimeSeries());
    }
    return out;
}

SafeVector<string> QuanSet::QuanNames()
{
    SafeVector<string> out;
    for (unordered_map<string,Quan>::iterator it=quans.begin(); it!=quans.end(); it++)
        out.push_back(it->first);
    return out;
}

bool QuanSet::DeleteConstituentRelatedProperties(const string &constituent_name)
{
    bool out = false;

    unordered_map<string,Quan>::iterator it=quans.begin();
    while (it != quans.end() ) {
        if (aquiutils::contains(it->first,":"))
        {
            if (aquiutils::split(it->first,':')[0] == constituent_name)
            {   it = quans.erase(it);
                out = true;
            }
            else
                ++it;
        }
        else {
            ++it;
        }
    }
    return out;
}

#ifdef QT_version
QStringList QuanSet::QQuanNames()
{
    QStringList out;
    for (map<string,Quan>::iterator it=quans.begin(); it!=quans.end(); it++)
        out<< QString::fromStdString(it->first);
    return out;
}

QuanSet::QuanSet(QJsonObject& object_types)
{
	parent = nullptr;
    //qDebug() << object_types;
	Name() = object_types.keys()[0].toStdString();
	ObjectType = "Entity";
	BlockLink = blocklink::entity;
	QStringList keys = object_types.keys();
    //qDebug() << keys;
    //qDebug() << object_types[keys[0]] << endl;
	//for each (QJsonObject it in object_types.value(object_types.keys()[0]).toArray())
	//{
		//qDebug() << it.value;
		/*if (it.key() == "icon")
			IconFileName() = it.value["filename"].asString();
		else if (it.key() == "typecategory")
			typecategory = (*object_types)[it.key().toStdString].asString();
		else if (it.key() == "type")
		{
			string _type = (*object_types)[it.key().asString()].asString();
			if (_type == "block")
			{
				BlockLink = blocklink::block;
				ObjectType = "Block";
			}
			else if (_type == "link")
			{
				BlockLink = blocklink::link;
				ObjectType = "Connector";
			}
			else if (_type == "source")
			{
				BlockLink = blocklink::source;
				ObjectType = "Source";
			}
			else
			{
				BlockLink = blocklink::entity;
				ObjectType = "Entity";
			}
		}
		else if (it.key() == "description")
			description = (*object_types)[it.key().asString()].asString();
		else
		{
			//cout<<it.key().asString()<<endl;
			Quan Q(it);
			Append(it.key().asString(), Q);
		}
	*/
	//}
}


#endif
string QuanSet::toCommand()
{
    string s;
    int i=0;
    for (unordered_map<string,Quan>::iterator it=quans.begin(); it!=quans.end(); it++)
    {
        if (it->second.AskFromUser())
        {
            if (i!=0) s += ",";
            s += it->second.toCommand();
            i++;
        }
    }
    return s;
}

string QuanSet::toCommandSetAsParam()
{
    string s;
    int i=0;
    for (unordered_map<string,Quan>::iterator it=quans.begin(); it!=quans.end(); it++)
    {
        if (it->second.AskFromUser())
        {
            if (it->second.GetParameterAssignedTo()!="")
            {
                if (i>0) s+= "\n";
                s+="setasparameter; object= " + quans["name"].GetProperty() + ", parametername= "  + it->second.GetParameterAssignedTo() + ", quantity= " + it->second.GetName();
                i++;
            }

        }
    }
    return s;
}


vector<string> QuanSet::quantitative_variable_list()
{
    vector<string> s;
    for (unordered_map<string,Quan>::iterator it=quans.begin(); it!=quans.end(); it++)
    {
        if (it->second.GetType() == Quan::_type::value || it->second.GetType() == Quan::_type::balance || it->second.GetType() == Quan::_type::constant || it->second.GetType() == Quan::_type::timeseries || it->second.GetType() == Quan::_type::expression || it->second.GetType() == Quan::_type::source)
        {
            s.push_back(it->second.GetName());
        }
    }
    return s;

}

bool QuanSet::RenameProperty(const string &oldname, const string &newname)
{
    RenameQuantity(oldname, newname);
    unordered_map<string,Quan>::iterator i = quans.find(oldname);

    if (i != quans.end())
    {
      Quan value = i->second;
      value.SetName(newname);
      quans.erase(i);
      DeleteInQuantityOrder(oldname);
      value.RenameQuantity(oldname,newname);
      return Append(newname, value);
    }
    else
        return false;


}

bool QuanSet::RenameInQuantityOrder(const string &oldname, const string &newname)
{
    for (unsigned int i=0; i<quantity_order.size(); i++)
        if (quantity_order[i]==oldname)
        {
            quantity_order[i]=newname;
            return true;
        }
    return false;
}

bool QuanSet::DeleteInQuantityOrder(const string& oldname)
{
    bool out = false;
    vector<string> new_quantity_order;
    for (unsigned int i = 0; i < quantity_order.size(); i++)
    {
        if (quantity_order[i] != oldname)
        {
            new_quantity_order.push_back(quantity_order[i]);
        }
        else
            out = true;
    }
    quantity_order = new_quantity_order;
    return out;
}

vector<string> QuanSet::AllConstituents()
{
    if (parent)
        return parent->AllConstituents();
    else
        return vector<string>();
}

vector<string> QuanSet::AllReactionParameters()
{
    if (parent)
        return parent->AllReactionParameters();
    else
        return vector<string>();

}

vector<string> QuanSet::ReviseQuanityOrder(const vector<string> &quantity, const string &constituent)
{
    vector<string> out = quantity_order;
    for (unsigned int j=0; j<quantity.size(); j++)
        for (unsigned int i=0; i<quantity_order.size(); i++)
        {
            if (quantity_order[i] == quantity[j])
                out[i] = constituent + ":" + quantity[j];
        }
    quantity_order = out;
    return out;
}

bool QuanSet::RenameQuantity(const string &oldname, const string &newname)
{
    bool out = false;
    for (unordered_map<string,Quan>::iterator it=quans.begin(); it!=quans.end(); it++)
        out = out || it->second.RenameQuantity(oldname, newname);
    return true;
}

void QuanSet::SetQuanPointers()
{
    if (parent!=nullptr)
    {
        for (unordered_map<string,Quan>::iterator it=quans.begin(); it!=quans.end(); it++)
            it->second.SetQuanPointers(parent);
    }
}
bool QuanSet::InitializePrecalcFunctions()
{
    bool out = true;

    for (unordered_map<string,Quan>::iterator it=quans.begin(); it!=quans.end(); it++)
        if (it->second.PreCalcFunction()->IndependentVariable()!="")
        {
            //qDebug()<<QString::fromStdString(it->first);
            out &= it->second.InitializePreCalcFunction();
        }

    return out;

}

bool QuanSet::RenameConstituents(const string &oldname, const string &newname)
{
    vector<string> oldfullname;
    vector<string> newfullname;
    for (unordered_map<string, Quan>::iterator it = quans.begin(); it != quans.end(); it++)
    {
        if (aquiutils::split(it->first,':').size()==2)
        {   if (aquiutils::split(it->first,':')[0]==oldname)
            {
                oldfullname.push_back(it->first);
                newfullname.push_back(newname + ":" + aquiutils::split(it->first,':')[1]);
            }
        }
    }
    bool succeed = true;
    for (unsigned int i=0; i<oldfullname.size(); i++)
    {
        succeed &= RenameProperty(oldfullname[i],newfullname[i]);
        if (quans.count(newfullname[i])>0)
        {   quans[newfullname[i]].Description() = newname + ":" + aquiutils::split(newfullname[i],':')[1];
            quans[newfullname[i]].Description(true) = newname + ":" + aquiutils::split(newfullname[i],':')[1];
        }
    }

    return succeed;
}

void QuanSet::CreateCPPcode(const string &source, const string header)
{
    ofstream sourcefile(source);
    ofstream headerfile(header);

    headerfile<<"class "<<this->Name()<<"\n";
    headerfile<<"{\n";
    headerfile<<"public:"<<"\n";
    headerfile<<"\t"<<this->Name()<<"();\n";
    headerfile<<"\tvirtual ~"<<this->Name()<<"();\n";
    headerfile<<"\t"<<this->Name()<<"(const "<<this->Name()<<"& other);\n";
    for (map<string,Quan>::iterator it=begin(); it!=end(); it++)
    {
        if (it->second.GetType()==Quan::_type::expression)
        {
            headerfile<<"\tdouble "<<it->first<<"() const;\n";
            sourcefile<<"double "<<this->name<<"::"<<it->first<<"() const;\n";
            sourcefile<<"{\n";

            sourcefile<<"}\n";
        }
    }
    for (map<string,Quan>::iterator it=begin(); it!=end(); it++)
    {
        if (it->second.GetType()==Quan::_type::value)
        {
            headerfile<<"\tdouble "<<it->first<<";\n";
        }
    }
    headerfile<<"}\n";
    headerfile.close();
    sourcefile.close();
}

QJsonObject QuanSet::toJson()
{
    QJsonObject out;
    for (map<string,Quan>::iterator it=begin(); it!=end(); it++)
    {
        if (it->second.AskFromUser())
            out[QString::fromStdString(it->first)] = QString::fromStdString(quans[it->first].GetProperty(it->second.GetType()!=Quan::_type::expression));
    }
    return out;
}


QJsonArray QuanSet::toJsonSetAsParameter()
{
    QJsonArray array;
    for (unordered_map<string,Quan>::iterator it=quans.begin(); it!=quans.end(); it++)
    {
        if (it->second.AskFromUser())
        {
            if (it->second.GetParameterAssignedTo()!="")
            {
                QJsonObject item;
                item["Object"] = QString::fromStdString(quans["name"].GetProperty());
                item["Parameter"] = QString::fromStdString(it->second.GetParameterAssignedTo());
                item["Quantity"] = QString::fromStdString(it->second.GetName());
                array.append(item);
            }
        }
    }
    return array;
}
