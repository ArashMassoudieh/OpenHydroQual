#ifndef ENUMS_H
#define ENUMS_H
#include "qcolor.h"

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

    };
};

struct objectColor
{
    QColor color1, color2, defaultColor;
};



#endif // ENUMS_H
