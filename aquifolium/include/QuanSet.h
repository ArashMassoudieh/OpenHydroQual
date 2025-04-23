#ifndef QUANSET_H
#define QUANSET_H

#ifdef QT_version
    #include <QStringList>
	#include <QJsonObject>
#endif

#include <map>
#include <unordered_map>
#include <map>
#include <Quan.h>

#define unordered_map map

class Object;

enum class blocklink {block, link, source, reaction, entity};

class QuanSet
{
    public:
        QuanSet();
        QuanSet(Json::ValueIterator &object_type);
        virtual ~QuanSet();
        QuanSet(const QuanSet& other);
        QuanSet& operator=(const QuanSet& other);
        bool Append(const string &s, const Quan &q);
        void Append(QuanSet &qset);
        size_t Count(const string &s) const {return quans.count(s);}
        Quan& operator[] (const string &s);
        Quan& GetVar(const string &s);
        Quan* GetVar(int i);
        Quan* GetVarAskable(int i);
        void UnUpdateAllValues();
        std::unordered_map<string,Quan>::iterator find(const string &name) {return quans.find(name);}
        std::unordered_map<string,Quan>::iterator end() {return quans.end();}
        std::unordered_map<string,Quan>::iterator begin() {return quans.begin();}
        std::unordered_map<string,Quan>::const_iterator const_end() const {return quans.cend();}
        std::unordered_map<string,Quan>::const_iterator const_begin() const {return quans.cbegin();}
        unsigned long size() {return quans.size();}
        unsigned long AskableSize();
        string &Description() 
        {
            return description;
        }
        string &IconFileName() {return iconfilename;}
        string &Name() {return name;}
        void ShowMessage(const string &msg);
        string ToString(int tabs=0);
        blocklink BlockLink;
        void SetParent(Object *p) {parent = p; SetAllParents();}
        void SetAllParents();
        Object *Parent() {return parent; }
        SafeVector<CTimeSeries<timeseriesprecision>*> TimeSeries();
        SafeVector<string> QuanNames();
        string toCommand();
        string toCommandSetAsParam();
        vector<string> quantitative_variable_list();
        bool RenameProperty(const string &oldname, const string &newname);
        bool RenameInQuantityOrder(const string &oldname, const string &newname);
        bool DeleteInQuantityOrder(const string& oldname);
        vector<string> AllConstituents();
        vector<string> AllReactionParameters();
        bool RenameQuantity(const string &oldname, const string &newname);
        bool RenameConstituents(const string &oldname, const string &newname);
        bool DeleteConstituentRelatedProperties(const string &constituent_name);
        bool Find(const string &s)
        {
            if (quans.find(s)!=quans.end())
                return true;
            else
                return false;
        }
        void SetQuanPointers();

#ifdef QT_version
        QStringList QQuanNames();
		QuanSet(QJsonObject& object_types);
#endif
        bool AppendError(const string &objectname, const string &cls, const string &funct, const string &description, const int &code);
		string ObjectType; 
		string& CategoryType() { return typecategory; }
        string& Normalizing_Quantity()
        {
            return normalizing_quantity;
        }
        vector<string>& Quantity_Order() {
            return quantity_order;
        }
        vector<string> ReviseQuanityOrder(const vector<string> &quantity, const string &constituent);
        bool InitializePrecalcFunctions();
        void CreateCPPcode(const string &source, const string header);
        QJsonObject toJson();
        QJsonArray toJsonSetAsParameter();
    protected:

    private:
        Object* parent = nullptr;
        string name = "";
        unordered_map<string, Quan> quans;
        string last_error = "";
        string description = "";
        string iconfilename = "";
        string typecategory = "";
        string normalizing_quantity="Storage";
        vector<string> quantity_order; 


};

#endif // QUANSET_H
