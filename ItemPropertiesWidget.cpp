
#include "ItemPropertiesWidget.h"
#include "ui_itempropertieswidget.h"

//#include "Panel.h"
//#include "MainView.h"
#include "ItemNavigator.h"

//#include <MetaModelLayer/MetaLayerItem.h>

#include <QSortFilterProxyModel>
#include <QToolButton>

//using Type = Variable::Type;

//////////////////////////////////////////////////////////////////////////
// TextButtonWidget

class TextButtonWidget: public QWidget {
public:
    using ClickedCallback = std::function<void(int /*property index*/)>;

    static constexpr auto default_height = 24; // Matches table view height

    TextButtonWidget(int prop_index, int height = default_height,
        QWidget* parent = nullptr)
        : QWidget{parent}, height_{height}, prop_index_{prop_index} {

        QSizePolicy size_policy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        size_policy.setHorizontalStretch(0);
        size_policy.setVerticalStretch(0);
        size_policy.setHeightForWidth(sizePolicy().hasHeightForWidth());

        setSizePolicy(size_policy);

        horizontal_layout_ = new QHBoxLayout();
        horizontal_layout_->setSpacing(0);

        horizontal_layout_->setSizeConstraint(QLayout::SetDefaultConstraint);
        horizontal_layout_->setContentsMargins(0, 0, 0, 0);

        horizontal_layout_->setObjectName(QString::fromUtf8("horizontalLayout"));

        horizontal_spacer_ = new QSpacerItem(40, 20, QSizePolicy::Expanding,
            QSizePolicy::Minimum);

        horizontal_layout_->addItem(horizontal_spacer_);

        tool_button_ = new QToolButton();
        tool_button_->setObjectName(QString::fromUtf8("toolButton"));
        horizontal_layout_->addWidget(tool_button_);

        setLayout(horizontal_layout_);

        QMetaObject::connectSlotsByName(this);

        connect(tool_button_, &QToolButton::clicked, this, [this]() {
            on_button_clicked();
        });
    }

    // Text
    void setText(QStringView text) { tool_button_->setText(text.toString()); }
    QString text() const { return tool_button_->text(); }

    // Icon
    void setIcon(QStringView icon_file) { 
        tool_button_->setIcon(QIcon{icon_file.toString()});
        tool_button_->setIconSize({height_, height_});
    }

    // Button
    QToolButton* button() { return tool_button_; }

    // Clicked callback
    void setClickedCallback(ClickedCallback callback = {})
        { clicked_callback_ = callback; }

private:
    void on_button_clicked() {
        if (clicked_callback_)
            clicked_callback_(prop_index_);
    }

    QHBoxLayout* horizontal_layout_ = nullptr;
    QSpacerItem* horizontal_spacer_ = nullptr;
    QToolButton* tool_button_       = nullptr;

    ClickedCallback clicked_callback_;

    int height_;
    int prop_index_;
};


//////////////////////////////////////////////////////////////////////////
// ItemPropertiesWidget

ItemPropertiesWidget::ItemPropertiesWidget(QWidget* parent):
    QFrame(parent),
    ui(new Ui::ItemPropertiesWidget) {
    ui->setupUi(this);

    // Setting special style for the filter line edit to look nicer in the
    // splitter.
    ui->edtFilter->setStyleSheet("QLineEdit {border-width: 0; border-style: solid; "
        "border-top-width: 1px; border-bottom-width: 0;}"
        "QLineEdit:hover {border-width: 0; border-style: solid; "
        "border-top-width: 1px; border-bottom-width: 0;}");

    setStyleSheet("QFrame {border: none;}");

    setupControls();
    _parent = parent;
    int width = parent->width();
    ui->edtFilter->hide();
    ui->pushButton->setEnabled(false);
    ui->Title->setWordWrap(true);
    ui->Title->setMaximumSize(width,200);
}

ItemPropertiesWidget::~ItemPropertiesWidget() {
    delete ui;
}

QTableView* ItemPropertiesWidget::tableView()
{
    return ui->tableView;
}

void ItemPropertiesWidget::setupControls() {
    // Tool buttons    

    // Navigate button


    //QMetaObject::connectSlotsByName(this);


}

void ItemPropertiesWidget::onNavigatorSet() {
    btnNavigate_->setIcon(icon());
}

/*
void ItemPropertiesWidget::setTableModel(PropModel* propmodel) {
    prop_model_ = propmodel;
    
    auto panel = qobject_cast<WaterwAIs::Panel*>(parent());
    Q_ASSERT(panel);

    if (!prop_model_) {
        ui->tableView->setModel(nullptr);
        ui->edtFilter->setText({});
        
        panel->setTitleText(u"Item");

        prop_proxy_model_ = nullptr;
        setNavigator();

        btnNavigate_->hide();
        return;
    }

    panel->setTitleText(prop_model_->itemLabel());

    prop_proxy_model_ = new QSortFilterProxyModel(this);
    prop_proxy_model_->setSourceModel(prop_model_);

    ui->tableView->setModel(prop_proxy_model_);
    ui->tableView->sortByColumn(0, Qt::AscendingOrder);
    ui->tableView->resizeColumnToContents(0);

    for (auto i = 0; i < prop_proxy_model_->rowCount(); i++) {
        auto item = prop_proxy_model_->index(i, 1);

        auto prop_index = prop_proxy_model_->data(item, Qt::UserRole).toInt();
        auto item_prop = prop_model_->getProperty(prop_index);

        if (item_prop.validTimeSeries()) {
            auto text_btn_widget = new TextButtonWidget{prop_index};
            text_btn_widget->setIcon(u":/Resources/chart.png");

            text_btn_widget->setClickedCallback([this](auto prop_index) {
                onButtonWidgetClicked(prop_index);
            });

            ui->tableView->setIndexWidget(item, text_btn_widget);
        }
    }
     
    btnNavigate_->show();
}

void ItemPropertiesWidget::on_edtFilter_textChanged(const QString& text) {
    if (prop_proxy_model_)
        prop_proxy_model_->setFilterWildcard(text + "*");
}



void ItemPropertiesWidget::onButtonWidgetClicked(int prop_index) {
    auto& prop = prop_model_->getProperty(prop_index);

    if (prop.validTimeSeries()) {
        emit showTimeSeries(prop_model_->itemName(), prop.name, prop.value,
            navigator());
    }        
}

void ItemPropertiesWidget::on_btnNavigate_clicked() {
   navigate();
}
*/
QSize ItemPropertiesWidget::minimumSizeHint() const {
    return {0, 0};
}

void ItemPropertiesWidget::SetTitleText(const QString &title)
{
    int width = _parent->width();
    ui->Title->setWordWrap(true);
    ui->Title->setMaximumSize(width,200);
    ui->Title->setText(title);
}
