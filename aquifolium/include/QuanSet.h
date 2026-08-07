/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 EnviroInformatics, LLC
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@enviroinformatics.co for details.
 */


#ifndef QUANSET_H
#define QUANSET_H

#ifdef Q_JSON_SUPPORT
    #include <QStringList>
	#include <QJsonObject>
#endif

#include <map>
#include <unordered_map>
#include <map>
#include <Quan.h>

#define unordered_map map

class Object;

enum class blocklink {block, link, source, reaction, entity, composite};

/**
 * @struct CompositeMember
 * @brief One member block of a composite type, as declared in the template
 *
 * Position is stored as an offset relative to the composite's own x/y so that
 * moving (or ungrouping) a composite lays its members out correctly.
 */
struct CompositeMember
{
    string type;
    double dx = 0, dy = 0;
    double width = 200, height = 200;
};

/**
 * @struct CompositeInternalLink
 * @brief One link internal to a composite type, connecting two of its members
 *
 * from/to are member keys (not instance-qualified names); they are resolved to
 * "<instance>__<member>" at instantiation time.
 */
struct CompositeInternalLink
{
    string type;
    string from;
    string to;
};

/**
 * @struct CompositePort
 * @brief Where an external link of a given type attaches to a composite
 *
 * Either side may be empty, meaning links of this type may not attach in that
 * direction. Looked up by link type with "*" as the fallback entry.
 */
struct CompositePort
{
    string from;
    string to;
};

class QuanSet: public unordered_map<string, Quan> 
{
    public:
        QuanSet();
        QuanSet(Json::ValueIterator &object_type);
        virtual ~QuanSet();
        QuanSet(const QuanSet& other);
        QuanSet& operator=(const QuanSet& other);
        bool Append(const string &s, const Quan &q);
        void Append(QuanSet &qset);
        size_t Count(const string &s) const {return count(s);}
        Quan& operator[] (const string &s);
        Quan& GetVar(const string &s);
        static Quan& missingQuanDummy();
        Quan* GetVar(int i);
        Quan* GetVarAskable(int i);
        void UnUpdateAllValues();
        std::unordered_map<string,Quan>::const_iterator const_end() const {return cend();}
        std::unordered_map<string,Quan>::const_iterator const_begin() const {return cbegin();}
        unsigned long size() const {return unordered_map<string, Quan>::size();}
        unsigned long AskableSize();
        string &Description() 
        {
            return description;
        }

        string Description() const
        {
            return description;
        }
        string &IconFileName() {return iconfilename;}
        string &Name() {return name;}
        void ShowMessage(const string &msg);
        string ToString(int tabs=0) const;
        blocklink BlockLink;
        void SetParent(Object *p) {parent = p; SetAllParents();}
        void SetAllParents();
        Object *Parent() {return parent; }
        SafeVector<TimeSeries<timeseriesprecision>*> GetTimeSeries(bool onlyprecip);
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
            if (find(s)!=end())
                return true;
            else
                return false;
        }
        void SetQuanPointers();

#ifdef Q_JSON_SUPPORT
        QStringList QQuanNames();
		QuanSet(QJsonObject& object_types);
        QJsonObject toJsonObjectFull() const;
#endif
        bool AppendError(const string &objectname, const string &cls, const string &funct, const string &description, const int &code);
		string ObjectType; 
		string& CategoryType() { return typecategory; }
        const string& CategoryType() const { return typecategory; }
        string& Normalizing_Quantity()
        {
            return normalizing_quantity;
        }
        vector<string>& Quantity_Order() {
            return quantity_order;
        }
        /**
         * @brief Member blocks declared by a composite type, keyed by member name
         *
         * Empty for every non-composite type. Ordered so that instantiation is
         * deterministic and member names are stable across sessions.
         */
        map<string, CompositeMember>& Members() { return members; }
        const map<string, CompositeMember>& Members() const { return members; }

        /**
         * @brief Links internal to a composite type, keyed by link name
         */
        map<string, CompositeInternalLink>& InternalLinks() { return internal_links; }
        const map<string, CompositeInternalLink>& InternalLinks() const { return internal_links; }

        /**
         * @brief External attachment points, keyed by link type ("*" = fallback)
         */
        map<string, CompositePort>& ExternalLinks() { return external_links; }
        const map<string, CompositePort>& ExternalLinks() const { return external_links; }

        vector<string> ReviseQuanityOrder(const vector<string> &quantity, const string &constituent);
        bool InitializePrecalcFunctions();
        void CreateCPPcode(const string &source, const string header);
        QJsonObject toJson(bool allvariables = false, bool calculatevalue = false);
        QJsonArray toJsonSetAsParameter();
    protected:

    private:
        Object* parent = nullptr;
        string name = "";
        string last_error = "";
        string description = "";
        string iconfilename = "";
        string typecategory = "";
        string normalizing_quantity="Storage";
        vector<string> quantity_order;
        map<string, CompositeMember> members;
        map<string, CompositeInternalLink> internal_links;
        map<string, CompositePort> external_links;


};

#endif // QUANSET_H
