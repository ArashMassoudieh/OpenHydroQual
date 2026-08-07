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

#include "Composite.h"
#include "System.h"
#include "Block.h"
#include "Link.h"
#include "Utilities.h"

Composite::Composite() : Object()
{
    SetObjectType(object_type::composite);
}

Composite::~Composite()
{

}

Composite::Composite(const Composite& other) : Object(other)
{
    members = other.members;
    internal_links = other.internal_links;
}

Composite& Composite::operator=(const Composite& other)
{
    if (this == &other) return *this;
    Object::operator=(other);
    members = other.members;
    internal_links = other.internal_links;
    return *this;
}

string Composite::MemberName(const string &memberkey) const
{
    return GetName() + COMPOSITE_MEMBER_SEPARATOR + memberkey;
}

bool Composite::Owns(const string &objectname) const
{
    for (unsigned int i=0; i<members.size(); i++)
        if (members[i] == objectname) return true;
    for (unsigned int i=0; i<internal_links.size(); i++)
        if (internal_links[i] == objectname) return true;
    return false;
}

bool Composite::Instantiate(System *sys)
{
    if (!sys)
        return false;

    QuanSet *model = sys->GetModel(GetType());
    if (!model)
    {
        sys->errorhandler.Append(GetName(),"Composite","Instantiate","Composite type '" + GetType() + "' does not exist!",18030);
        return false;
    }

    members.clear();
    internal_links.clear();

    bool success = true;

    // Members first: the internal links below refer to them by name.
    for (map<string, CompositeMember>::const_iterator it=model->Members().cbegin(); it!=model->Members().cend(); it++)
    {
        string membername = MemberName(it->first);
        if (sys->object(membername)!=nullptr)
        {
            sys->errorhandler.Append(GetName(),"Composite","Instantiate","An object named '" + membername + "' already exists!",18031);
            success = false;
            continue;
        }
        if (sys->GetModel(it->second.type)==nullptr)
        {
            sys->errorhandler.Append(GetName(),"Composite","Instantiate","Member '" + it->first + "' of composite '" + GetName() + "' has unknown type '" + it->second.type + "'",18032);
            success = false;
            continue;
        }
        Block B;
        B.SetType(it->second.type);
        B.SetName(membername);
        sys->AddBlock(B);
        Block *blk = sys->block(membername);
        if (!blk)
        {
            sys->errorhandler.Append(GetName(),"Composite","Instantiate","Member '" + membername + "' could not be created",18033);
            success = false;
            continue;
        }
        blk->SetName(membername);
        blk->AssignRandomPrimaryKey();
        members.push_back(membername);
    }

    for (map<string, CompositeInternalLink>::const_iterator it=model->InternalLinks().cbegin(); it!=model->InternalLinks().cend(); it++)
    {
        string linkname = MemberName(it->first);
        if (sys->object(linkname)!=nullptr)
        {
            sys->errorhandler.Append(GetName(),"Composite","Instantiate","An object named '" + linkname + "' already exists!",18031);
            success = false;
            continue;
        }
        if (sys->GetModel(it->second.type)==nullptr)
        {
            sys->errorhandler.Append(GetName(),"Composite","Instantiate","Internal link '" + it->first + "' of composite '" + GetName() + "' has unknown type '" + it->second.type + "'",18034);
            success = false;
            continue;
        }
        Link L;
        L.SetType(it->second.type);
        L.SetName(linkname);
        if (!sys->AddLink(L, MemberName(it->second.from), MemberName(it->second.to)))
        {
            sys->errorhandler.Append(GetName(),"Composite","Instantiate","Internal link '" + linkname + "' could not be created: " + sys->lasterror(),18035);
            success = false;
            continue;
        }
        Link *lnk = sys->link(linkname);
        if (lnk)
        {
            lnk->SetName(linkname);
            lnk->AssignRandomPrimaryKey();
        }
        internal_links.push_back(linkname);
    }

    ApplyGeometry(sys);
    SetMemberGeometryVisible(sys, false);

    // Constituents may already exist, in which case the members just picked up
    // their derived quantities and the composite needs matching properties.
    sys->AddCompositeConstituentRelatedProperties();

    if (!Propagate(sys))
        success = false;

    return success;
}

void Composite::ApplyGeometry(System *sys)
{
    if (!sys) return;
    QuanSet *model = sys->GetModel(GetType());
    if (!model) return;

    double x0 = GetVal("x");
    double y0 = GetVal("y");

    for (map<string, CompositeMember>::const_iterator it=model->Members().cbegin(); it!=model->Members().cend(); it++)
    {
        Block *blk = sys->block(MemberName(it->first));
        if (!blk) continue;
        blk->SetVal("x", x0 + it->second.dx);
        blk->SetVal("y", y0 + it->second.dy);
        blk->SetVal("_width", it->second.width);
        blk->SetVal("_height", it->second.height);
    }
}

void Composite::SetMemberGeometryVisible(System *sys, bool visible)
{
    if (!sys) return;

    static const char *geometry[4] = {"x", "y", "_width", "_height"};
    for (unsigned int i=0; i<members.size(); i++)
    {
        Block *blk = sys->block(members[i]);
        if (!blk) continue;
        for (int g=0; g<4; g++)
            if (blk->HasQuantity(geometry[g]))
                blk->Variable(geometry[g])->AskFromUser() = visible;
    }
}

bool Composite::Propagate(System *sys)
{
    if (!sys) return false;

    bool success = true;

    for (unordered_map<string, Quan>::iterator it=GetVars()->begin(); it!=GetVars()->end(); it++)
    {
        const map<string, string> &maps = it->second.ApplyTo();
        for (map<string, string>::const_iterator mp=maps.cbegin(); mp!=maps.cend(); mp++)
        {
            vector<string> target = aquiutils::split(mp->first, COMPOSITE_QUANTITY_SEPARATOR);
            if (target.size()!=2)
            {
                sys->errorhandler.Append(GetName(),"Composite","Propagate","Mapping target '" + mp->first + "' must have the form <member>#<quantity>",18036);
                success = false;
                continue;
            }
            string ownername = MemberName(target[0]);
            Object *owner = sys->object(ownername);
            if (!owner)
            {
                sys->errorhandler.Append(GetName(),"Composite","Propagate","Mapping target '" + mp->first + "' refers to '" + ownername + "', which does not exist",18037);
                success = false;
                continue;
            }
            if (!owner->HasQuantity(target[1]))
            {
                sys->errorhandler.Append(GetName(),"Composite","Propagate","Object '" + ownername + "' has no property '" + target[1] + "'",18038);
                success = false;
                continue;
            }

            Quan::_type targettype = owner->Variable(target[1])->GetType();
            if (targettype == Quan::_type::value || targettype == Quan::_type::balance || targettype == Quan::_type::constant)
            {
                // Numeric target: the mapping is an expression over this
                // composite's own properties.
                Expression expression(mp->second);
                owner->SetVal(target[1], expression.calc(this, Expression::timing::past));
            }
            else
            {
                // Everything else (source, timeseries, string, boolean) is a
                // pass-through of the referenced composite property's value.
                if (!HasQuantity(mp->second))
                {
                    sys->errorhandler.Append(GetName(),"Composite","Propagate","Property '" + target[1] + "' of '" + ownername + "' is not numeric, so its mapping must be the bare name of a composite property; '" + mp->second + "' is not one",18039);
                    success = false;
                    continue;
                }
                owner->SetProperty(target[1], Variable(mp->second)->GetProperty(), true, false);
            }
        }
    }

    return success;
}

string Composite::ResolvePort(const string &linktype, Expression::loc end)
{
    System *sys = GetParent();
    if (!sys) return "";
    QuanSet *model = sys->GetModel(GetType());
    if (!model) return "";

    const map<string, CompositePort> &ports = model->ExternalLinks();

    map<string, CompositePort>::const_iterator it = ports.find(linktype);
    if (it == ports.cend())
        it = ports.find("*");
    if (it == ports.cend())
        return "";

    string memberkey = (end == Expression::loc::source) ? it->second.from : it->second.to;
    if (memberkey == "")
        return "";

    return MemberName(memberkey);
}
