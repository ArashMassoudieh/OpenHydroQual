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
