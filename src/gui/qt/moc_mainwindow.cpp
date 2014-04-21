/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.0.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.0.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[16];
    char stringdata[260];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_MainWindow_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10),
QT_MOC_LITERAL(1, 11, 12),
QT_MOC_LITERAL(2, 24, 0),
QT_MOC_LITERAL(3, 25, 1),
QT_MOC_LITERAL(4, 27, 16),
QT_MOC_LITERAL(5, 44, 3),
QT_MOC_LITERAL(6, 48, 3),
QT_MOC_LITERAL(7, 52, 37),
QT_MOC_LITERAL(8, 90, 4),
QT_MOC_LITERAL(9, 95, 37),
QT_MOC_LITERAL(10, 133, 32),
QT_MOC_LITERAL(11, 166, 28),
QT_MOC_LITERAL(12, 195, 22),
QT_MOC_LITERAL(13, 218, 24),
QT_MOC_LITERAL(14, 243, 5),
QT_MOC_LITERAL(15, 249, 9)
    },
    "MainWindow\0onUpdateText\0\0v\0onSetLanComboBox\0"
    "src\0det\0on_srcLanComboBox_currentIndexChanged\0"
    "arg1\0on_detLanComboBox_currentIndexChanged\0"
    "on_inputLineEdit_editingFinished\0"
    "on_inputLineEdit_textChanged\0"
    "on_queryButton_clicked\0on_indexListView_clicked\0"
    "index\0onAppExit\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   59,    2, 0x0a,
       4,    3,   62,    2, 0x0a,
       7,    1,   69,    2, 0x08,
       9,    1,   72,    2, 0x08,
      10,    0,   75,    2, 0x08,
      11,    1,   76,    2, 0x08,
      12,    0,   79,    2, 0x08,
      13,    1,   80,    2, 0x08,
      15,    0,   83,    2, 0x08,

 // slots: parameters
    QMetaType::Void, QMetaType::VoidStar,    3,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::VoidStar,    5,    6,    3,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QModelIndex,   14,
    QMetaType::Void,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MainWindow *_t = static_cast<MainWindow *>(_o);
        switch (_id) {
        case 0: _t->onUpdateText((*reinterpret_cast< void*(*)>(_a[1]))); break;
        case 1: _t->onSetLanComboBox((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< void*(*)>(_a[3]))); break;
        case 2: _t->on_srcLanComboBox_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->on_detLanComboBox_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->on_inputLineEdit_editingFinished(); break;
        case 5: _t->on_inputLineEdit_textChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->on_queryButton_clicked(); break;
        case 7: _t->on_indexListView_clicked((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 8: _t->onAppExit(); break;
        default: ;
        }
    }
}

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow.data,
      qt_meta_data_MainWindow,  qt_static_metacall, 0, 0}
};


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
