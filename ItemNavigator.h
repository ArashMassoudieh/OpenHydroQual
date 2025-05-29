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


#ifndef ITEMNAVIGATOR_H_B338CD7F1E150082
#define ITEMNAVIGATOR_H_B338CD7F1E150082

#include <QIcon>
#include <memory>


//////////////////////////////////////////////////////////////////////////
// ItemNavigator

class ItemNavigator {
public:
    using Ptr = std::shared_ptr<ItemNavigator>;

    ItemNavigator() = default;
    virtual ~ItemNavigator() = default;

    // Returns item's icon
    virtual QIcon itemIcon() const;

    // Performs navigating to the item
    virtual void navigate() = 0;
};


//////////////////////////////////////////////////////////////////////////
// ItemNavigatorHolder

class ItemNavigatorHolder {
public:
    using ItemNavigatorPtr = ItemNavigator::Ptr;

    // Item navigator
    void setNavigator(ItemNavigatorPtr item_navigator = {});
    ItemNavigatorPtr navigator() { return item_navigator_; }

    // Icon
    QIcon icon() const { return icon_; }

    // Navigates to the item
    void navigate()
        { if (item_navigator_) item_navigator_->navigate(); }

protected:
    // Called when navigator was set
    virtual void onNavigatorSet() {}

private:
    ItemNavigatorPtr item_navigator_;
    QIcon icon_;
};


//////////////////////////////////////////////////////////////////////////
#endif // ITEMNAVIGATOR_H_B338CD7F1E150082
