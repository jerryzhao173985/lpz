# Virtual Functions Missing Override Keywords in Configurator Component

## Summary

The configurator component has **32 virtual functions** that are missing the `override` keyword. These are distributed across several classes:

## Classes with Most Missing Overrides

### 1. **QAbstractConfigurableTileWidget** (17 functions)
This base class has the most virtual functions without override:
- Event handlers: `enterEvent()`, `leaveEvent()`, `mouseMoveEvent()`, `mousePressEvent()`, `mouseReleaseEvent()`
- Custom virtuals: `setVisible()`, `isVisible()`, `contains()`, `setGridPos()`, `getGridPos()`, etc.
- Destructor: `~QAbstractConfigurableTileWidget()`

### 2. **QConfigurableWidget** (10 functions)
- Event handlers: `enterEvent()`, `leaveEvent()`, `mousePressEvent()`, `mouseDoubleClickEvent()`, `dragEnterEvent()`, `dragMoveEvent()`, `dropEvent()`, `dragLeaveEvent()`
- Callback method: `doOnCallBack()`
- Destructor: `~QConfigurableWidget()`

### 3. **QConfigurator** (2 functions)
- Event handlers: `closeEvent()`, `enterEvent()`
These are overriding QMainWindow methods and should have `override`.

### 4. **Dialog Classes** (3 destructors)
- `QConfigurableLoadSaveDialog::~QConfigurableLoadSaveDialog()`
- `QConfigurableSetBoundsDialog::~QConfigurableSetBoundsDialog()`
- `QDummyConfigurableTileWidget::~QDummyConfigurableTileWidget()`

### 5. **QLogViewWidget** (1 destructor)
- `~QLogViewWidget()`

## Functions That Need Override

### Event Handlers (need override)
These are overriding Qt base class methods:
- `enterEvent(QEnterEvent*)` - overrides QWidget
- `leaveEvent(QEvent*)` - overrides QWidget
- `mousePressEvent(QMouseEvent*)` - overrides QWidget
- `mouseReleaseEvent(QMouseEvent*)` - overrides QWidget
- `mouseMoveEvent(QMouseEvent*)` - overrides QWidget
- `mouseDoubleClickEvent(QMouseEvent*)` - overrides QWidget
- `dragEnterEvent(QDragEnterEvent*)` - overrides QWidget
- `dragMoveEvent(QDragMoveEvent*)` - overrides QWidget
- `dropEvent(QDropEvent*)` - overrides QWidget
- `dragLeaveEvent(QDragLeaveEvent*)` - overrides QWidget
- `closeEvent(QCloseEvent*)` - overrides QWidget
- `setVisible(bool)` - overrides QWidget
- `isVisible()` - overrides QWidget method (but returns bool, not void)

### Custom Virtual Functions (don't need override)
These appear to be new virtual functions introduced by the classes:
- `getName()`, `contains()`, `setGridPos()`, `getGridPos()`, `getConfigurableName()`
- `setInCollapseMode()`, `updatePaletteChanged()`
- `sl_resize()` - slot function
- `doOnCallBack()` - from Callbackable interface

### Destructors
Virtual destructors in derived classes should have `override` in C++11+:
- All destructors in classes that inherit from base classes with virtual destructors

## Recommendations

1. **High Priority**: Add `override` to all Qt event handler methods (enterEvent, leaveEvent, etc.)
2. **Medium Priority**: Add `override` to destructors in derived classes
3. **Low Priority**: Consider marking non-overriding virtual functions as `virtual` only in base classes

The QAbstractConfigurableTileWidget and QConfigurableWidget classes have the most missing override keywords and should be prioritized for fixing.