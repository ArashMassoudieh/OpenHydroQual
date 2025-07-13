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


#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include <vector>
#include <map>
#include "Quan.h"
#include "MetaModel.h"
#include "memory"
#ifdef Q_version
#include <qdebug.h>
#endif
#include "ErrorHandler.h"

using namespace std;

enum class object_type {none, block, link, source, parameter, objective_function, reaction, reaction_parameter, constituent, observation};

class Object
{
    public:
        Object();
        virtual ~Object();
        Object(const Object& other);
        Object& operator=(const Object& other);
        double CalcVal(const string& s, const Timing &tmg=Timing::past);
        double GetVal(const string& s, const Timing &tmg=Timing::past, bool limit=false);
        double GetVal(const string& var, const string& consttnt, const Timing &tmg, bool limit=false);
        double GetVal(Quan* quan,const Timing &tmg, bool limit);
        bool AddQnantity(const string &name,const Quan &Q);
        bool SetQuantities(MetaModel &m, const string& typ);
        bool SetQuantities(MetaModel *m, const string& typ );
        bool SetQuantities(System *sys, const string& typ );
        bool SetQuantities(QuanSet &Q);
        bool HasQuantity(const string &q);
        bool SetVal(const string& s, double value, const Timing &tmg = Timing::both);
        bool SetVal(const string& s, const string & value, const Timing &tmg = Timing::both);
        double GetProperty(const string& s) {
            if (Variable(s) != nullptr)
            {
                return Variable(s)->GetVal();
                //qDebug() << QString::fromStdString(s) << ": " << var.GetVar(s).GetVal();
            }
            else
            {
                //qDebug() << "NullPtr";
                return 0;
            }
        }
        System *GetParent() const
        {
            return parent;
        }
        string GetName() const;
        void SetDefaults();
        virtual bool SetName(const string &_name, bool setprop=true);
        Object* GetConnectedBlock(Expression::loc l);
        void SetConnectedBlock(Expression::loc l, const string &blockname);
        void AppendError(const string &s);
        void SetParent(System *s);
        Quan* CorrespondingFlowVariable(const string &s);

        Quan* Variable(const string &s);
        Quan* Variable(const string &variable, const string &constituent);
        void SetType(const string &typ) {type = typ;}
        string GetType() {return type;}
        unsigned int s_Block_No() {return s_Block_no; }
        void SetSBlockNo(int i) { s_Block_no = i; }
        unsigned int e_Block_No() {return e_Block_no; }
        void SetEBlockNo(int i) { e_Block_no = i; }
        void Set_s_Block(Object *O) { s_Block = O; }
        void Set_e_Block(Object *O) { e_Block = O; }
        Object* Get_s_Block() { return s_Block; }
        Object* Get_e_Block() { return e_Block; }
		bool Renew(const string &variable);
		bool Update(const string &variable);
		bool CalcExpressions(const Timing& tmg);
        bool EstablishExpressionStructure();
        bool VerifyQuans(ErrorHandler *errorhandler);
        SafeVector<TimeSeries<timeseriesprecision>*> GetTimeSeries(bool onlyprecip = false) {return var.GetTimeSeries(onlyprecip);}
        string TypeCategory() {return GetVars()->CategoryType();}
		QuanSet* GetVars()
            {
                return &var;
            }
        vector<Quan> GetCopyofAllQuans();
        void SetOutflowLimitFactor(const double &val, const Timing &tmg)
		{
			if (tmg == Timing::past)
                outflowlimitfactor_past = val; // max(0.0,val);
			else
                outflowlimitfactor_current = val; //max(0.0,val);
		}
        double GetOutflowLimitFactor(const Timing &tmg)
		{

			if (tmg == Timing::past)
                return outflowlimitfactor_past;
			else
                return outflowlimitfactor_current;

		}
        void SetLimitedOutflow(bool x);
        bool GetLimitedOutflow() {return limitoutflow;}
		void SetVariableParents();
        void ShowMessage(const string &msg);
        System* Parent() {if (parent!=nullptr) return parent; else return nullptr;}
        void SetAllParents();
        bool SetProperty(const string &prop, const string &value, bool force_value=false, bool check_criteria=true);
        string toString(int _tabs=0);
        void SetPrimaryKey(const string &prmkey) {primary_key = prmkey;}
        string GetPrimaryKey() {return primary_key;}
        void AssignRandomPrimaryKey();
        string toCommand();
        QJsonObject toJson(bool allvariables = false, bool calculatevalue = false);
        string toCommandSetAsParam();
        vector<string> ItemswithOutput();
        vector<string> quantitative_variable_list() {return var.quantitative_variable_list();}
        unique_ptr<vector<string>> &operators();
        unique_ptr<vector<string>> &functions();
        string& lasterror() {
            return last_error;
        }
        vector<string>& QuantitOrder() { return var.Quantity_Order();  }
        void UnUpdateAllValues() { var.UnUpdateAllValues(); }
        bool RenameProperty(const string &oldname, const string &newname)
        {
            return var.RenameProperty(oldname, newname);
        }
        bool RenameConstituents(const string &oldname, const string &newname);
        bool CalculateInitialValues();
        vector<string> AllConstituents();
        vector<string> AllReactionParameters();
        bool InitializePrecalcFunctions();
        void MakeTimeSeriesUniform(const double &increment);
        void SetQuanPointers();
        bool CopyStateVariablesFrom(Object* obj);
        unordered_map<string, Quan*> AllSourceParameters();
        void SetCurrentCorrespondingSource(const string s) {current_corresponding_source = s; }
        void SetCurrentCorrespondingConstituent(const string s) {current_corresponding_constituent = s; }
        string GetCurrentCorrespondingSource() {return current_corresponding_source; }
        string GetCurrentCorrespondingConstituent() {return current_corresponding_constituent; }
        object_type ObjectType() {return Object_Type;}
        void SetObjectType(object_type typ) {Object_Type = typ;}

    protected:

    private:
        string current_corresponding_source="";
        string current_corresponding_constituent="";
        QuanSet var;
        vector<string> errors;
        string last_error;
        bool last_operation_success;
        map<string, string> setting;
        System *parent = nullptr;
        string name;
        Object *s_Block=nullptr;
        Object *e_Block=nullptr;
        unsigned int s_Block_no, e_Block_no;
        string type;
        object_type Object_Type;
        double outflowlimitfactor_past = 1;
		double outflowlimitfactor_current = 1;
        bool limitoutflow = false;
        string primary_key = "";

};

#endif // OBJECT_H
