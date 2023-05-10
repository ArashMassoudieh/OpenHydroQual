
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


