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

#ifndef COMPOSITE_H
#define COMPOSITE_H

#include "Object.h"

class System;
class Block;
class Link;

/**
 * @brief Separator between a composite instance name and a member name
 *
 * "B1" + "Pond" -> "B1__Pond". Deliberately not '.', which Expression reserves
 * for the ".s"/".e"/".v" endpoint suffixes.
 */
#define COMPOSITE_MEMBER_SEPARATOR "__"

/**
 * @brief Separator between an object name and one of its quantities
 *
 * "B1__Pond" + "depth" -> "B1__Pond#depth". Deliberately not ':', which is
 * already the constituent scoping separator inside quantity names.
 */
#define COMPOSITE_QUANTITY_SEPARATOR '#'

/**
 * @class Composite
 * @brief A group of blocks and links that the GUI and the file format treat as one object
 *
 * A Composite owns a set of member blocks and the links between them. The
 * computational engine never sees the composite: its members are ordinary
 * Block and Link objects living in the System's normal containers, and every
 * solver, parameter-estimation and output path treats them individually.
 *
 * What the composite changes is:
 *   - persistence: the model file carries a single "create composite" line
 *     instead of one line per member, and the members are regenerated from
 *     the template on load;
 *   - the diagram: one Node is drawn for the composite rather than one per
 *     member.
 *
 * Composite derives from Object, so it carries its own QuanSet holding the
 * group-level properties declared by its type. Those properties drive member
 * quantities through the mapping expressions in Quan::ApplyTo(), which are
 * evaluated against the composite itself (Expression resolves symbols by name
 * against the object it is handed). Consequently a mapping expression may only
 * reference other composite-level properties.
 *
 * Members are never edited individually: the composite's exposed properties
 * fully determine them. A user who needs a property the type does not expose
 * ungroups the composite, after which it is plain blocks and links.
 */
class Composite : public Object
{
public:
    Composite();
    virtual ~Composite();
    Composite(const Composite& other);
    Composite& operator=(const Composite& other);

    /**
     * @brief Creates the member blocks and internal links in the system
     * @param sys The system to create the members in
     * @return true on success; false if the type is unknown or a member could not be created
     *
     * Member names are "<composite name>__<member key>". Geometry is derived
     * from the composite's own x/y plus the per-member dx/dy offsets declared
     * by the type. Propagate() is run at the end so members start out
     * consistent with the group-level property values.
     *
     * @note The composite must already have its name, type and parent set.
     */
    bool Instantiate(System *sys);

    /**
     * @brief Pushes group-level property values down onto member quantities
     * @param sys The owning system
     * @return true if every mapping was applied
     *
     * For each composite property carrying an "applyto" map, each target
     * "<member>#<quantity>" is written. Numeric targets receive the mapping
     * expression evaluated against this composite; every other target type
     * (source, timeseries, string, boolean) receives the referenced property's
     * raw string value.
     *
     * Must be called after instantiation, after any group-level property edit,
     * and from System::ApplyParameters() so that calibration reaches members.
     */
    bool Propagate(System *sys);

    /**
     * @brief Finds the member an external link of the given type attaches to
     * @param linktype The type of the external link
     * @param end Whether the composite is the source or the destination
     * @return The instance-qualified member name, or "" if the type may not attach
     *
     * Looks up the type in the composite type's external_links map, falling
     * back to the "*" entry.
     */
    string ResolvePort(const string &linktype, Expression::loc end);

    /**
     * @brief Instance-qualified names of the member blocks
     */
    const vector<string>& MemberNames() const {return members;}
    vector<string>& MemberNames() {return members;}

    /**
     * @brief Instance-qualified names of the links internal to this composite
     */
    const vector<string>& InternalLinkNames() const {return internal_links;}
    vector<string>& InternalLinkNames() {return internal_links;}

    /**
     * @brief Whether the named block or link belongs to this composite
     */
    bool Owns(const string &objectname) const;

    /**
     * @brief Builds the instance-qualified name of a member from its type-level key
     */
    string MemberName(const string &memberkey) const;

    /**
     * @brief Copies the composite's position and size onto its members
     * @param sys The owning system
     *
     * Member positions are the composite's x/y plus the offsets declared by
     * the type, so that moving the composite - or ungrouping it - leaves the
     * members laid out as the type author intended.
     */
    void ApplyGeometry(System *sys);

private:
    vector<string> members;
    vector<string> internal_links;
};

#endif // COMPOSITE_H
