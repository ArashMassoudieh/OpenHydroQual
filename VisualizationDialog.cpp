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

#include "VisualizationDialog.h"
#include "VisualizationGraphicsView.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsPolygonItem>
#include <QFileDialog>
#include <QMessageBox>
#include <QWheelEvent>
#include <QScrollBar>
#include <QPainter>
#include <QSvgGenerator>
#include <cmath>
#include <algorithm>
#include <limits>

VisualizationDialog::VisualizationDialog(System* sys, QWidget *parent)
    : QDialog(parent), system(sys), scene(nullptr),
      currentTime(0.0), minTime(0.0), maxTime(1.0)
{
    setWindowTitle("Model Visualization");
    resize(1200, 800);
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);
    setWindowFlags(Qt::Window);

    setupUI();
    populatePropertyComboBox();
    setupTimeSlider();
}

VisualizationDialog::~VisualizationDialog()
{
    // Disconnect all signals to prevent callbacks after deletion
    disconnect();

    // Clear scene items before destruction
    if (scene)
    {
        scene->clear();
    }

    // Null out the system pointer to catch any post-destruction access
    system = nullptr;
}

void VisualizationDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Top control panel
    QHBoxLayout* controlLayout = new QHBoxLayout();

    QLabel* propertyLabel = new QLabel("Property:", this);
    propertyComboBox = new QComboBox(this);
    propertyComboBox->setMinimumWidth(300);

    resetViewButton = new QPushButton("Reset View", this);
    exportButton = new QPushButton("Export as SVG", this);

    controlLayout->addWidget(propertyLabel);
    controlLayout->addWidget(propertyComboBox);
    controlLayout->addStretch();
    controlLayout->addWidget(resetViewButton);
    controlLayout->addWidget(exportButton);

    mainLayout->addLayout(controlLayout);

    // Time slider panel
    QHBoxLayout* timeLayout = new QHBoxLayout();

    QLabel* timeSliderLabel = new QLabel("Time:", this);
    minTimeLabel = new QLabel("0.0", this);
    minTimeLabel->setMinimumWidth(60);
    minTimeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    timeSlider = new QSlider(Qt::Horizontal, this);
    timeSlider->setMinimum(0);
    timeSlider->setMaximum(TIME_SLIDER_RESOLUTION);
    timeSlider->setValue(0);
    timeSlider->setTickPosition(QSlider::TicksBelow);
    timeSlider->setTickInterval(TIME_SLIDER_RESOLUTION / 10);

    maxTimeLabel = new QLabel("1.0", this);
    maxTimeLabel->setMinimumWidth(60);

    timeLabel = new QLabel("t = 0.0", this);
    timeLabel->setMinimumWidth(100);
    timeLabel->setStyleSheet("QLabel { font-weight: bold; }");

    timeLayout->addWidget(timeSliderLabel);
    timeLayout->addWidget(minTimeLabel);
    timeLayout->addWidget(timeSlider);
    timeLayout->addWidget(maxTimeLabel);
    timeLayout->addWidget(timeLabel);

    mainLayout->addLayout(timeLayout);

    // Graphics view and legend side-by-side
    QHBoxLayout* viewLayout = new QHBoxLayout();

    // Graphics view on the left
    scene = new QGraphicsScene(this);
    graphicsView = new VisualizationGraphicsView(scene, this);
    viewLayout->addWidget(graphicsView, 1); // Stretch factor 1

    // Create legend widget on the right
    legendWidget = new QWidget(this);
    legendWidget->setFixedWidth(150);
    legendWidget->setStyleSheet("QWidget { background-color: white; border: 1px solid #ccc; }");

    QVBoxLayout* legendLayout = new QVBoxLayout(legendWidget);
    legendLayout->setSpacing(10);
    legendLayout->setContentsMargins(10, 10, 10, 10);

    legendTitleLabel = new QLabel("Legend", legendWidget);
    legendTitleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    legendTitleLabel->setAlignment(Qt::AlignCenter);

    legendMaxLabel = new QLabel("Max: 1.0", legendWidget);
    legendMaxLabel->setStyleSheet("font-size: 12px;");
    legendMaxLabel->setAlignment(Qt::AlignCenter);

    legendMidLabel = new QLabel("Mid: 0.5", legendWidget);
    legendMidLabel->setStyleSheet("font-size: 12px;");
    legendMidLabel->setAlignment(Qt::AlignCenter);

    legendMinLabel = new QLabel("Min: 0.0", legendWidget);
    legendMinLabel->setStyleSheet("font-size: 12px;");
    legendMinLabel->setAlignment(Qt::AlignCenter);

    legendLayout->addWidget(legendTitleLabel);
    legendLayout->addSpacing(20);
    legendLayout->addWidget(legendMaxLabel);
    legendLayout->addStretch();

    // Add color gradient visualization
    QLabel* gradientLabel = new QLabel(legendWidget);
    gradientLabel->setObjectName("gradientLabel"); // Set object name for later lookup
    gradientLabel->setFixedSize(80, 200);
    QPixmap gradientPixmap(80, 200);
    QPainter painter(&gradientPixmap);
    for (int i = 0; i < 200; ++i)
    {
        double value = 1.0 - (i / 199.0);
        QColor color = getColorForValue(value, 0.0, 1.0);
        painter.setPen(color);
        painter.drawLine(0, i, 80, i);
    }
    painter.end();
    gradientLabel->setPixmap(gradientPixmap);
    gradientLabel->setAlignment(Qt::AlignCenter);
    legendLayout->addWidget(gradientLabel, 0, Qt::AlignCenter);

    legendLayout->addStretch();
    legendLayout->addWidget(legendMidLabel);
    legendLayout->addStretch();
    legendLayout->addWidget(legendMinLabel);
    legendLayout->addSpacing(10);

    viewLayout->addWidget(legendWidget);
    mainLayout->addLayout(viewLayout);

    // Bottom legend panel - simplified now
    QHBoxLayout* bottomLayout = new QHBoxLayout();
    minValueLabel = new QLabel("", this); // Keep for compatibility but hide
    maxValueLabel = new QLabel("", this); // Keep for compatibility but hide
    minValueLabel->setVisible(false);
    maxValueLabel->setVisible(false);
    bottomLayout->addStretch();
    mainLayout->addLayout(bottomLayout);

    // Connect signals
    connect(propertyComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VisualizationDialog::onPropertyChanged);
    connect(timeSlider, &QSlider::valueChanged, this, &VisualizationDialog::onTimeChanged);
    connect(resetViewButton, &QPushButton::clicked, this, &VisualizationDialog::onResetView);
    connect(exportButton, &QPushButton::clicked, this, &VisualizationDialog::onExport);
}

void VisualizationDialog::populatePropertyComboBox()
{
    if (!system)
        return;

    properties = system->GetOutputProperties();

    propertyComboBox->clear();

    for (size_t i = 0; i < properties.size(); ++i)
    {
        QString displayText = QString::fromStdString(properties[i].second); // Description
        propertyComboBox->addItem(displayText);
    }

    if (properties.size() > 0)
    {
        currentProperty = properties[0].first;
        drawVisualization();
    }
}

bool VisualizationDialog::hasSimulationData() const
{
    if (!system)
        return false;

    return (system->GetOutputs().size() > 0 && system->GetOutputs().maxnumpoints() > 0);
}

void VisualizationDialog::setupTimeSlider()
{
    if (!hasSimulationData())
    {
        // No simulation data - disable time slider
        timeSlider->setEnabled(false);
        minTimeLabel->setText("No data");
        maxTimeLabel->setText("No data");
        timeLabel->setText("t = N/A");
        currentTime = 0.0;
        minTime = 0.0;
        maxTime = 1.0;
        return;
    }

    // Get time range from outputs
    minTime = system->GetOutputs().mintime();
    maxTime = system->GetOutputs().maxtime();
    currentTime = minTime;

    // Update labels
    minTimeLabel->setText(QString::number(minTime, 'f', 2));
    maxTimeLabel->setText(QString::number(maxTime, 'f', 2));
    timeLabel->setText(QString("t = %1").arg(currentTime, 0, 'f', 4));

    // Enable slider
    timeSlider->setEnabled(true);
    timeSlider->setValue(0);
}

double VisualizationDialog::getValueAtTime(const string& objectName,
                                           const string& propertyName,
                                           double defaultValue)
{
    if (!hasSimulationData())
    {
        // No simulation data - fall back to current property value
        return defaultValue;
    }

    // Construct the series name: "ObjectName_PropertyName"
    string seriesName = objectName + "_" + propertyName;

    // Check if this series exists in outputs
    if (!system->GetOutputs().Contains(seriesName))
    {
        return defaultValue;
    }

    // Interpolate value at current time
    try
    {
        const TimeSeries<outputtimeseriesprecision>& series =
            system->GetOutputs()[seriesName];

        // Check if series has data
        if (series.size() == 0)
            return defaultValue;

        // Interpolate at current time
        return series.interpol(currentTime);
    }
    catch (...)
    {
        // If any error occurs, return default value
        return defaultValue;
    }
}

void VisualizationDialog::onPropertyChanged(int index)
{
    if (index >= 0 && index < static_cast<int>(properties.size()))
    {
        currentProperty = properties[index].first;

        // Get GLOBAL min/max across all time series
        double minValue = std::numeric_limits<double>::max();
        double maxValue = std::numeric_limits<double>::lowest();
        bool foundAny = false;

        if (hasSimulationData())
        {
            // Find global min/max from entire time series for blocks
            for (unsigned int i = 0; i < system->BlockCount(); ++i)
            {
                Block* block = system->block(i);
                if (block)
                {
                    string seriesName = block->GetName() + "_" + currentProperty;
                    if (system->GetOutputs().Contains(seriesName))
                    {
                        const TimeSeries<outputtimeseriesprecision>& series =
                            system->GetOutputs()[seriesName];
                        if (series.size() > 0)
                        {
                            double seriesMin = series.minVal();
                            double seriesMax = series.maxVal();
                            minValue = std::min(minValue, seriesMin);
                            maxValue = std::max(maxValue, seriesMax);
                            foundAny = true;
                        }
                    }
                }
            }

            // Find global min/max from entire time series for links
            for (unsigned int i = 0; i < system->LinksCount(); ++i)
            {
                Link* link = system->link(i);
                if (link)
                {
                    string seriesName = link->GetName() + "_" + currentProperty;
                    if (system->GetOutputs().Contains(seriesName))
                    {
                        const TimeSeries<outputtimeseriesprecision>& series =
                            system->GetOutputs()[seriesName];
                        if (series.size() > 0)
                        {
                            double seriesMin = series.minVal();
                            double seriesMax = series.maxVal();
                            minValue = std::min(minValue, seriesMin);
                            maxValue = std::max(maxValue, seriesMax);
                            foundAny = true;
                        }
                    }
                }
            }
        }
        else
        {
            // No simulation data - use current property values
            getPropertyRange(currentProperty, minValue, maxValue);
            foundAny = true;
        }

        // Fallback if no data found
        if (!foundAny)
        {
            minValue = 0.0;
            maxValue = 1.0;
        }

        // Handle identical values
        if (maxValue - minValue < 1e-10)
        {
            minValue -= 0.5;
            maxValue += 0.5;
        }

        // Update legend with global min/max
        createLegend(minValue, maxValue);

        // Redraw visualization
        drawVisualization();
    }
}

void VisualizationDialog::onTimeChanged(int value)
{
    // Convert slider position (0-1000) to actual time
    double normalizedTime = static_cast<double>(value) / TIME_SLIDER_RESOLUTION;
    currentTime = minTime + normalizedTime * (maxTime - minTime);

    // Update time label
    timeLabel->setText(QString("t = %1").arg(currentTime, 0, 'f', 4));

    // Redraw visualization with new time
    drawVisualization();
}

void VisualizationDialog::onResetView()
{
    graphicsView->resetTransform();
    graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void VisualizationDialog::onExport()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "Export Visualization", "", "SVG Files (*.svg)");

    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".svg", Qt::CaseInsensitive))
        fileName += ".svg";

    QSvgGenerator generator;
    generator.setFileName(fileName);
    generator.setSize(scene->sceneRect().size().toSize());
    generator.setViewBox(scene->sceneRect());
    generator.setTitle("Model Visualization");
    generator.setDescription("Water Quality Model Visualization");

    QPainter painter;
    painter.begin(&generator);
    scene->render(&painter);
    painter.end();

    QMessageBox::information(this, "Export Successful",
        "Visualization exported successfully to:\n" + fileName);
}

void VisualizationDialog::drawVisualization()
{
    if (!system || currentProperty.empty())
        return;

    // Clear the scene
    scene->clear();

    // Get property range for color mapping
    double minValue = 0.0;
    double maxValue = 1.0;
    getPropertyRange(currentProperty, minValue, maxValue);

    // Update legend
    minValueLabel->setText(QString("Min: %1").arg(minValue, 0, 'g', 4));
    maxValueLabel->setText(QString("Max: %1").arg(maxValue, 0, 'g', 4));

    bool useSimulationData = hasSimulationData();

    // Draw all blocks
    for (unsigned int i = 0; i < system->BlockCount(); ++i)
    {
        Block* block = system->block(i);
        if (!block)
            continue;

        // Get block position
        double x = DEFAULT_BLOCK_WIDTH * i; // Default if property doesn't exist
        double y = DEFAULT_BLOCK_HEIGHT * i;
        double width = DEFAULT_BLOCK_WIDTH;
        double height = DEFAULT_BLOCK_HEIGHT;

        // Try to get x, y from properties
        if (block->HasQuantity("x"))
            x = block->GetVal("x", Expression::timing::present);
        if (block->HasQuantity("y"))
            y = block->GetVal("y", Expression::timing::present);
        if (block->HasQuantity("_width"))
            width = block->GetVal("_width", Expression::timing::present);
        if (block->HasQuantity("_height"))
            height = block->GetVal("_height", Expression::timing::present);

        // Get property value for color coding
        double value = 0.0;
        QColor fillColor = Qt::gray; // Default color

        bool hasProperty = false;

        if (useSimulationData)
        {
            // Get value from simulation outputs at current time
            value = getValueAtTime(block->GetName(), currentProperty,
                                  std::numeric_limits<double>::quiet_NaN());
            if (!std::isnan(value))
            {
                fillColor = getColorForValue(value, minValue, maxValue);
                hasProperty = true;
            }
        }
        else if (block->HasQuantity(currentProperty))
        {
            // Fall back to current property value (no simulation)
            value = block->GetVal(currentProperty, Expression::timing::present);
            fillColor = getColorForValue(value, minValue, maxValue);
            hasProperty = true;
        }

        // If block doesn't have this property, keep it grey (fillColor already set to Qt::gray)

        // Draw the block rectangle
        QGraphicsRectItem* rectItem = scene->addRect(x, y, width, height,
            QPen(Qt::black, 2), QBrush(fillColor));
        rectItem->setToolTip(QString::fromStdString(
            block->GetName() + "\n" + currentProperty + ": " +
            std::to_string(value)));

        // Add block name text
        QGraphicsTextItem* textItem = scene->addText(
            QString::fromStdString(block->GetName()));
        textItem->setDefaultTextColor(Qt::black);

        // Center the text in the block
        QRectF textRect = textItem->boundingRect();
        textItem->setPos(x + (width - textRect.width()) / 2.0,
                        y + (height - textRect.height()) / 2.0);
    }

    // Draw all links
    for (unsigned int i = 0; i < system->LinksCount(); ++i)
    {
        Link* link = system->link(i);
        if (!link)
            continue;

        Block* sourceBlock = dynamic_cast<Block*>(link->GetConnectedBlock(Expression::loc::source));
        Block* destBlock = dynamic_cast<Block*>(link->GetConnectedBlock(Expression::loc::destination));

        if (!sourceBlock || !destBlock)
            continue;

        // Get source and destination positions
        double x1 = DEFAULT_BLOCK_WIDTH * 0;
        double y1 = DEFAULT_BLOCK_HEIGHT * 0;
        double w1 = DEFAULT_BLOCK_WIDTH;
        double h1 = DEFAULT_BLOCK_HEIGHT;

        double x2 = DEFAULT_BLOCK_WIDTH * 1;
        double y2 = DEFAULT_BLOCK_HEIGHT * 1;
        double w2 = DEFAULT_BLOCK_WIDTH;
        double h2 = DEFAULT_BLOCK_HEIGHT;

        if (sourceBlock->HasQuantity("x"))
            x1 = sourceBlock->GetVal("x", Expression::timing::present);
        if (sourceBlock->HasQuantity("y"))
            y1 = sourceBlock->GetVal("y", Expression::timing::present);
        if (sourceBlock->HasQuantity("_width"))
            w1 = sourceBlock->GetVal("_width", Expression::timing::present);
        if (sourceBlock->HasQuantity("_height"))
            h1 = sourceBlock->GetVal("_height", Expression::timing::present);

        if (destBlock->HasQuantity("x"))
            x2 = destBlock->GetVal("x", Expression::timing::present);
        if (destBlock->HasQuantity("y"))
            y2 = destBlock->GetVal("y", Expression::timing::present);
        if (destBlock->HasQuantity("_width"))
            w2 = destBlock->GetVal("_width", Expression::timing::present);
        if (destBlock->HasQuantity("_height"))
            h2 = destBlock->GetVal("_height", Expression::timing::present);

        // Calculate centers of blocks
        double centerX1 = x1 + w1 / 2.0;
        double centerY1 = y1 + h1 / 2.0;
        double centerX2 = x2 + w2 / 2.0;
        double centerY2 = y2 + h2 / 2.0;

        // Calculate edge points (where line intersects rectangle edges)
        double dx = centerX2 - centerX1;
        double dy = centerY2 - centerY1;
        double angle = std::atan2(dy, dx);

        // Find intersection points on rectangle edges
        double startX = centerX1;
        double startY = centerY1;
        double endX = centerX2;
        double endY = centerY2;

        // Approximate edge points (simplified - connect from edges of rectangles)
        if (std::abs(dx) > std::abs(dy))
        {
            // Horizontal dominant
            if (dx > 0)
            {
                startX = x1 + w1;
                endX = x2;
            }
            else
            {
                startX = x1;
                endX = x2 + w2;
            }
            startY = centerY1;
            endY = centerY2;
        }
        else
        {
            // Vertical dominant
            if (dy > 0)
            {
                startY = y1 + h1;
                endY = y2;
            }
            else
            {
                startY = y1;
                endY = y2 + h2;
            }
            startX = centerX1;
            endX = centerX2;
        }

        // Get link color based on property value if applicable
        QColor linkColor = Qt::darkGray;
        double linkValue = 0.0;

        if (useSimulationData)
        {
            // Get value from simulation outputs at current time
            linkValue = getValueAtTime(link->GetName(), currentProperty, 0.0);
            linkColor = getColorForValue(linkValue, minValue, maxValue);
        }
        else if (link->HasQuantity(currentProperty))
        {
            // Fall back to current property value (no simulation)
            linkValue = link->GetVal(currentProperty, Expression::timing::present);
            linkColor = getColorForValue(linkValue, minValue, maxValue);
        }

        // Draw the line
        QGraphicsLineItem* lineItem = scene->addLine(startX, startY, endX, endY,
            QPen(linkColor, LINK_WIDTH));
        lineItem->setToolTip(QString::fromStdString(
            link->GetName() + "\n" + currentProperty + ": " +
            std::to_string(linkValue)));

        // Draw arrow head
        double arrowAngle = std::atan2(endY - startY, endX - startX);
        QPointF arrowP1 = QPointF(endX - ARROW_SIZE * std::cos(arrowAngle - M_PI / 6),
                                  endY - ARROW_SIZE * std::sin(arrowAngle - M_PI / 6));
        QPointF arrowP2 = QPointF(endX - ARROW_SIZE * std::cos(arrowAngle + M_PI / 6),
                                  endY - ARROW_SIZE * std::sin(arrowAngle + M_PI / 6));

        QPolygonF arrowHead;
        arrowHead << QPointF(endX, endY) << arrowP1 << arrowP2;
        QGraphicsPolygonItem* arrow = scene->addPolygon(arrowHead,
            QPen(linkColor, 1), QBrush(linkColor));
        arrow->setToolTip(lineItem->toolTip());
    }

    // Calculate and set scene bounds
    double minX, minY, maxX, maxY;
    calculateBounds(minX, minY, maxX, maxY);

    // Add some padding
    double padding = 50.0;
    scene->setSceneRect(minX - padding, minY - padding,
                       maxX - minX + 2 * padding,
                       maxY - minY + 2 * padding);

    // Fit the view
    graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);

}

QColor VisualizationDialog::getColorForValue(double value, double minValue, double maxValue)
{
    // Handle edge cases
    if (maxValue <= minValue)
        return QColor(128, 128, 255); // Medium blue

    // Normalize value to 0-1 range
    double normalized = (value - minValue) / (maxValue - minValue);
    normalized = std::max(0.0, std::min(1.0, normalized)); // Clamp to [0, 1]

    // Create color gradient from blue (cold/low) to red (hot/high)
    // Blue -> Cyan -> Green -> Yellow -> Red
    int r, g, b;

    if (normalized < 0.25)
    {
        // Blue to Cyan
        double t = normalized / 0.25;
        r = 0;
        g = static_cast<int>(255 * t);
        b = 255;
    }
    else if (normalized < 0.5)
    {
        // Cyan to Green
        double t = (normalized - 0.25) / 0.25;
        r = 0;
        g = 255;
        b = static_cast<int>(255 * (1 - t));
    }
    else if (normalized < 0.75)
    {
        // Green to Yellow
        double t = (normalized - 0.5) / 0.25;
        r = static_cast<int>(255 * t);
        g = 255;
        b = 0;
    }
    else
    {
        // Yellow to Red
        double t = (normalized - 0.75) / 0.25;
        r = 255;
        g = static_cast<int>(255 * (1 - t));
        b = 0;
    }

    return QColor(r, g, b);
}

void VisualizationDialog::getPropertyRange(const string& propertyName, double& minValue, double& maxValue)
{
    minValue = std::numeric_limits<double>::max();
    maxValue = std::numeric_limits<double>::lowest();

    bool foundAny = false;
    bool useSimulationData = hasSimulationData();

    if (useSimulationData)
    {
        // Get min/max from simulation outputs at current time
        // Check blocks
        for (unsigned int i = 0; i < system->BlockCount(); ++i)
        {
            Block* block = system->block(i);
            if (block)
            {
                double value = getValueAtTime(block->GetName(), propertyName,
                                             std::numeric_limits<double>::quiet_NaN());
                if (!std::isnan(value))
                {
                    minValue = std::min(minValue, value);
                    maxValue = std::max(maxValue, value);
                    foundAny = true;
                }
            }
        }

        // Check links
        for (unsigned int i = 0; i < system->LinksCount(); ++i)
        {
            Link* link = system->link(i);
            if (link)
            {
                double value = getValueAtTime(link->GetName(), propertyName,
                                             std::numeric_limits<double>::quiet_NaN());
                if (!std::isnan(value))
                {
                    minValue = std::min(minValue, value);
                    maxValue = std::max(maxValue, value);
                    foundAny = true;
                }
            }
        }
    }
    else
    {
        // Fall back to current property values (no simulation)
        // Check blocks
        for (unsigned int i = 0; i < system->BlockCount(); ++i)
        {
            Block* block = system->block(i);
            if (block && block->HasQuantity(propertyName))
            {
                double value = block->GetVal(propertyName, Expression::timing::present);
                minValue = std::min(minValue, value);
                maxValue = std::max(maxValue, value);
                foundAny = true;
            }
        }

        // Check links
        for (unsigned int i = 0; i < system->LinksCount(); ++i)
        {
            Link* link = system->link(i);
            if (link && link->HasQuantity(propertyName))
            {
                double value = link->GetVal(propertyName, Expression::timing::present);
                minValue = std::min(minValue, value);
                maxValue = std::max(maxValue, value);
                foundAny = true;
            }
        }
    }

    // If no values found, use defaults
    if (!foundAny)
    {
        minValue = 0.0;
        maxValue = 1.0;
    }

    // If all values are the same, add small range
    if (maxValue - minValue < 1e-10)
    {
        minValue -= 0.5;
        maxValue += 0.5;
    }
}

void VisualizationDialog::calculateBounds(double& minX, double& minY, double& maxX, double& maxY)
{
    minX = std::numeric_limits<double>::max();
    minY = std::numeric_limits<double>::max();
    maxX = std::numeric_limits<double>::lowest();
    maxY = std::numeric_limits<double>::lowest();

    for (unsigned int i = 0; i < system->BlockCount(); ++i)
    {
        Block* block = system->block(i);
        if (!block)
            continue;

        double x = DEFAULT_BLOCK_WIDTH * i;
        double y = DEFAULT_BLOCK_HEIGHT * i;
        double width = DEFAULT_BLOCK_WIDTH;
        double height = DEFAULT_BLOCK_HEIGHT;

        if (block->HasQuantity("x"))
            x = block->GetVal("x", Expression::timing::present);
        if (block->HasQuantity("y"))
            y = block->GetVal("y", Expression::timing::present);
        if (block->HasQuantity("_width"))
            width = block->GetVal("_width", Expression::timing::present);
        if (block->HasQuantity("_height"))
            height = block->GetVal("_height", Expression::timing::present);

        minX = std::min(minX, x);
        minY = std::min(minY, y);
        maxX = std::max(maxX, x + width);
        maxY = std::max(maxY, y + height);
    }

    // If no blocks, use defaults
    if (system->BlockCount() == 0)
    {
        minX = 0;
        minY = 0;
        maxX = 800;
        maxY = 600;
    }
}

void VisualizationDialog::createLegend(double minValue, double maxValue)
{
    // Update legend labels
    legendMinLabel->setText(QString("Min:\n%1").arg(minValue, 0, 'g', 4));
    legendMaxLabel->setText(QString("Max:\n%1").arg(maxValue, 0, 'g', 4));
    legendMidLabel->setText(QString("Mid:\n%1").arg((minValue + maxValue) / 2.0, 0, 'g', 4));

    // Update gradient pixmap
    QLabel* gradientLabel = legendWidget->findChild<QLabel*>("gradientLabel");
    if (gradientLabel)
    {
        QPixmap gradientPixmap(80, 200);
        QPainter painter(&gradientPixmap);
        for (int i = 0; i < 200; ++i)
        {
            double normalizedPos = i / 199.0;
            double value = minValue + (maxValue - minValue) * (1.0 - normalizedPos);
            QColor color = getColorForValue(value, minValue, maxValue);
            painter.setPen(color);
            painter.drawLine(0, i, 80, i);
        }
        painter.end();
        gradientLabel->setPixmap(gradientPixmap);
    }
}
