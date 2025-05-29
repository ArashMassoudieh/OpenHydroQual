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



#include "ItemNavigator.h"

#include <QApplication>
#include <QStyle>


//////////////////////////////////////////////////////////////////////////
// ItemNavigator

QIcon ItemNavigator::itemIcon() const {
    return QApplication::style()->standardIcon(QStyle::SP_CommandLink);
}

//////////////////////////////////////////////////////////////////////////
// ItemNavigatorHolder

void ItemNavigatorHolder::setNavigator(ItemNavigatorPtr item_navigator) {
    item_navigator_ = item_navigator;

    icon_ = QIcon{};

    if (item_navigator_)
        icon_ = item_navigator_->itemIcon();

    if (icon_.isNull())
        icon_ = QApplication::style()->standardIcon(QStyle::SP_CommandLink);

    onNavigatorSet();
}


