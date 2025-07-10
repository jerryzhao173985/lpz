// Qt3Support Removal Example for guilogger
// This shows the exact code changes needed to migrate from Qt3 to Qt5/Qt6

// ===== BEFORE (Qt3Support) =====
// guilogger.h (old version with Qt3Support)
/*
#include <q3scrollview.h>
#include <q3canvas.h>
#include <q3listview.h>
#include <q3valuelist.h>
#include <q3dict.h>

class GuiLogger : public QMainWindow {
    Q3ScrollView* scrollView;
    Q3Canvas* canvas;
    Q3ListView* channelList;
    Q3ValueList<ChannelData> channels;
    Q3Dict<PlotInfo> plotDict;
    
    void setupUI() {
        scrollView = new Q3ScrollView(this);
        canvas = new Q3Canvas(800, 600);
        scrollView->setCanvas(canvas);
        
        channelList = new Q3ListView(this);
        channelList->addColumn("Channel");
        channelList->addColumn("Value");
    }
};
*/

// ===== AFTER (Qt5/Qt6 Compatible) =====
// guilogger.h (modernized version)
#ifndef GUILOGGER_H
#define GUILOGGER_H

#include <QMainWindow>
#include <QScrollArea>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTreeWidget>
#include <QList>
#include <QHash>
#include <memory>

// Forward declarations
class ChannelData;
class PlotInfo;
class PlotWidget;

class GuiLogger : public QMainWindow {
    Q_OBJECT

public:
    explicit GuiLogger(QWidget* parent = nullptr);
    ~GuiLogger() override = default;

private:
    // Modern Qt5/Qt6 widgets
    QScrollArea* scrollArea;
    QGraphicsScene* scene;
    QGraphicsView* graphicsView;
    QTreeWidget* channelList;
    
    // Use modern STL containers with Qt types
    QList<ChannelData> channels;
    QHash<QString, std::unique_ptr<PlotInfo>> plotDict;
    
    // Setup methods
    void setupUI();
    void setupMenus();
    void setupConnections();
    
private slots:
    void updateChannelDisplay();
    void onChannelSelectionChanged();
    void addNewChannel(const QString& name, double value);
};

// guilogger.cpp (modernized implementation)
#include "guilogger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTreeWidgetItem>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>

GuiLogger::GuiLogger(QWidget* parent) 
    : QMainWindow(parent)
    , scrollArea(nullptr)
    , scene(nullptr)
    , graphicsView(nullptr)
    , channelList(nullptr) {
    
    setupUI();
    setupMenus();
    setupConnections();
}

void GuiLogger::setupUI() {
    // Create central widget and main layout
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    auto* mainLayout = new QHBoxLayout(centralWidget);
    
    // Create splitter for resizable panes
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    mainLayout->addWidget(splitter);
    
    // Left pane: Channel list (replaces Q3ListView)
    channelList = new QTreeWidget(this);
    channelList->setHeaderLabels(QStringList() << "Channel" << "Value");
    channelList->setRootIsDecorated(false);
    channelList->setAlternatingRowColors(true);
    channelList->setSortingEnabled(true);
    splitter->addWidget(channelList);
    
    // Right pane: Graphics view (replaces Q3Canvas/Q3ScrollView)
    graphicsView = new QGraphicsView(this);
    scene = new QGraphicsScene(0, 0, 800, 600, this);
    graphicsView->setScene(scene);
    graphicsView->setRenderHint(QPainter::Antialiasing);
    graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    splitter->addWidget(graphicsView);
    
    // Set initial splitter sizes (30% for list, 70% for graphics)
    splitter->setSizes(QList<int>() << 300 << 700);
    
    // Status bar
    statusBar()->showMessage("Ready");
}

void GuiLogger::setupMenus() {
    // File menu
    QMenu* fileMenu = menuBar()->addMenu("&File");
    
    QAction* openAction = fileMenu->addAction("&Open...");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, [this]() {
        // Implementation for file open
    });
    
    fileMenu->addSeparator();
    
    QAction* exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    // View menu
    QMenu* viewMenu = menuBar()->addMenu("&View");
    
    QAction* zoomInAction = viewMenu->addAction("Zoom &In");
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    connect(zoomInAction, &QAction::triggered, this, [this]() {
        graphicsView->scale(1.2, 1.2);
    });
    
    QAction* zoomOutAction = viewMenu->addAction("Zoom &Out");
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOutAction, &QAction::triggered, this, [this]() {
        graphicsView->scale(0.8, 0.8);
    });
}

void GuiLogger::setupConnections() {
    // Connect channel selection changes
    connect(channelList, &QTreeWidget::itemSelectionChanged,
            this, &GuiLogger::onChannelSelectionChanged);
}

void GuiLogger::updateChannelDisplay() {
    // Update all channel values in the tree widget
    for (int i = 0; i < channelList->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = channelList->topLevelItem(i);
        if (i < channels.size()) {
            item->setText(1, QString::number(channels[i].value));
        }
    }
}

void GuiLogger::onChannelSelectionChanged() {
    // Handle channel selection
    QList<QTreeWidgetItem*> selected = channelList->selectedItems();
    
    // Clear previous highlights in scene
    for (auto* item : scene->items()) {
        item->setSelected(false);
    }
    
    // Highlight selected channels in the graphics view
    for (auto* item : selected) {
        QString channelName = item->text(0);
        // Find and highlight corresponding graphics items
        // (implementation depends on your plotting system)
    }
}

void GuiLogger::addNewChannel(const QString& name, double value) {
    // Add to data structure
    ChannelData newChannel;
    newChannel.name = name;
    newChannel.value = value;
    channels.append(newChannel);
    
    // Add to tree widget
    auto* item = new QTreeWidgetItem(channelList);
    item->setText(0, name);
    item->setText(1, QString::number(value));
    
    // Create plot info if needed
    if (!plotDict.contains(name)) {
        plotDict[name] = std::make_unique<PlotInfo>(name);
    }
}

// ===== Migration Notes =====
/*
Key changes made:
1. Q3ScrollView -> QScrollArea or QGraphicsView
2. Q3Canvas -> QGraphicsScene + QGraphicsView
3. Q3ListView -> QTreeWidget (more flexible) or QListView + QStandardItemModel
4. Q3ValueList -> QList (Qt container)
5. Q3Dict -> QHash or std::unordered_map

Additional improvements:
- Use of smart pointers (std::unique_ptr) for better memory management
- Modern signal/slot syntax with lambdas
- Proper use of layouts instead of manual positioning
- Better separation of concerns
- Use of override keyword for virtual functions
- Explicit constructors
- Use of nullptr instead of 0/NULL
*/

#endif // GUILOGGER_H