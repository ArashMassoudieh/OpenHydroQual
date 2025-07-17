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
#include <Quan.h>

#define unordered_map map

class Object;

/// Enum indicating linkage category for the object type
enum class blocklink { block, link, source, reaction, entity };

/**
 * @class QuanSet
 * @brief A container for managing named Quan objects along with their metadata, evaluation order, and parent context.
 */
class QuanSet
{
public:
    QuanSet(); ///< Default constructor
    QuanSet(Json::ValueIterator& object_type); ///< Constructor from JSON definition
	Quan& Var(const std::string& s); //<< Getter for Quan by name
    virtual ~QuanSet(); ///< Destructor
    QuanSet(const QuanSet& other); ///< Copy constructor
    QuanSet& operator=(const QuanSet& other); ///< Copy assignment

    bool Append(const string& s, const Quan& q); ///< Add a new Quan
    void Append(QuanSet& qset); ///< Add all Quans from another set

    size_t Count(const string& s) const; ///< Number of Quans with given name
    Quan& operator[](const string& s); ///< Direct access (unsafe if missing)
    Quan& GetVar(const string& s); ///< Safe access (raises error if missing)
    Quan* GetVar(int i); ///< Get by index
    Quan* GetVarAskable(int i); ///< Get ith user-askable Quan

    void UnUpdateAllValues(); ///< Clear update flags

    std::unordered_map<string, Quan>::iterator find(const string& name); ///< Lookup by name
    std::unordered_map<string, Quan>::iterator begin(); ///< Begin iterator
    std::unordered_map<string, Quan>::iterator end(); ///< End iterator
    std::unordered_map<string, Quan>::const_iterator const_end() const; ///< Const end iterator
    std::unordered_map<string, Quan>::const_iterator const_begin() const; ///< Const begin iterator

    unsigned long size(); ///< Total number of Quans
    unsigned long AskableSize(); ///< Count of askable Quans

    string& Description(); ///< Get description
    string& IconFileName(); ///< Get icon path
    string& Name(); ///< Get object name
    void ShowMessage(const string& msg); ///< Output if not silent
    string ToString(int tabs = 0); ///< Serialize as string

    blocklink BlockLink; ///< Role of the object (block, link, etc.)

    void SetParent(Object* p); ///< Set parent object
    void SetAllParents(); ///< Assign parent to all Quans
    Object* Parent(); ///< Access parent

    SafeVector<TimeSeries<timeseriesprecision>*> GetTimeSeries(bool onlyprecip = false); ///< Time series references
    SafeVector<string> QuanNames(); ///< List of all Quan names

    string toCommand(); ///< Serialize askable Quans as CLI string
    string toCommandSetAsParam(); ///< Serialize askables as parameters

    vector<string> quantitative_variable_list(); ///< Names of quantitative variables
    bool RenameProperty(const string& oldname, const string& newname); ///< Rename a quantity
    bool RenameInQuantityOrder(const string& oldname, const string& newname); ///< Rename in order
    bool DeleteInQuantityOrder(const string& oldname); ///< Remove from order

    vector<string> AllConstituents(); ///< All known constituents
    vector<string> AllReactionParameters(); ///< All known parameters

    bool RenameQuantity(const string& oldname, const string& newname); ///< Rename in expression
    bool RenameConstituents(const string& oldname, const string& newname); ///< Rename scoped constituent
    bool DeleteConstituentRelatedProperties(const string& constituent_name); ///< Remove scoped Quans

    bool Find(const string& s); ///< Test if Quan exists
    void SetQuanPointers(); ///< Link pointers for expression eval

#ifdef Q_JSON_SUPPORT
    QStringList QQuanNames(); ///< Qt list of quantity names
    QuanSet(QJsonObject& object_types); ///< Load from JSON object
    QJsonObject toJson(bool allvariables = false, bool calculatevalue = false); ///< Export to JSON
    QJsonArray toJsonSetAsParameter(); ///< Export parameter bindings
    QJsonObject EquationsToJson() const; ///< Export expressions
#endif

    bool AppendError(const string& objectname, const string& cls, const string& funct, const string& description, const int& code); ///< Log error

    string ObjectType; ///< Descriptive label of object type
    string& CategoryType(); ///< Reference to type category
    string& Normalizing_Quantity(); ///< Name of normalization variable
    vector<string>& Quantity_Order(); ///< Ordered list of quantities

    vector<string> ReviseQuanityOrder(const vector<string>& quantity, const string& constituent); ///< Prefix quantities
    bool InitializePrecalcFunctions(); ///< Setup precalculated data
    void CreateCPPcode(const string& source, const string header); ///< Auto-generate class boilerplate

private:
    Object* parent = nullptr;
    string name = "";
    unordered_map<string, Quan> quans;
    string last_error = "";
    string description = "";
    string iconfilename = "";
    string typecategory = "";
    string normalizing_quantity = "Storage";
    vector<string> quantity_order;
};

#endif // QUANSET_H

