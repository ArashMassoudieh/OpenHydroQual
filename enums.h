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


#ifndef ENUMS_H
#define ENUMS_H
#include "qcolor.h"

enum Operation_Modes{ Draw_Connector, Node1_selected, Pan, NormalMode, resizeNode, ZoomWindow };
enum class Object_Types { Void, Block, Connector, RayLine };
enum mListReadStatus{fileNotValid, readSuccessfully, readBefore, errorInContents};
enum corners{ none, topleft, topright, bottomleft, bottomright };
enum edgesides{ noside, topside, leftside, bottomside, rightside };


namespace CustomRoleCodes {
    enum Role {
        TypeRole =							2700,
        InputMethodRole =					2702,
        DefaultValuesListRole =				2704,
        VariableTypeRole =					2706,
        VariableNameRole =					2707,
        VariableNameToolTipRole =			2708,
        warningConditionRole =				2709,
        warningErrorRole =					2710,
        warningErrorDescRole =				2711,
        DescriptionCodeRole =				2712,
        experimentDependentRole =			2713,
        differentValuesRole =				2714,
        differentValuesMultiObjectRole =	2716,
        UnitRole =							2802,
        defaultUnitRole =					2803,
        UnitsListRole =						2804,
        allUnitsRole =						2805,
        TreeItemType =						2901,
        TreeParentItemType =				2902,
        XStringEditRole =					3001,
        setParamRole =						3002,
        fullFileNameRole =					30704,
        saveIndex =							3101,
        loadIndex =							3102,
        loadIndexandInputMethodRole =		3103,
        loadIndexandDefaultValuesListRole = 3104,
        loadIndexandVariableTypeRole =		3105,
        allowableWordsRole =				3201,
        referedObjectRole =                 3301,
        EstimateCode =                      3401,
        ObjectName =                        3402,
        VariableName =                      3403,
    };
};

struct objectColor
{
    QColor color1, color2, defaultColor;
};



#endif // ENUMS_H
