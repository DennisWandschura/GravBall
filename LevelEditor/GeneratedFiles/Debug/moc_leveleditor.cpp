/****************************************************************************
** Meta object code from reading C++ file 'leveleditor.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../leveleditor.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'leveleditor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_LevelEditor_t {
    QByteArrayData data[13];
    char stringdata0[183];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_LevelEditor_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_LevelEditor_t qt_meta_stringdata_LevelEditor = {
    {
QT_MOC_LITERAL(0, 0, 11), // "LevelEditor"
QT_MOC_LITERAL(1, 12, 16), // "onUpdatePosition"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 15), // "onUpdateHalfDim"
QT_MOC_LITERAL(4, 46, 14), // "onUpdateRadius"
QT_MOC_LITERAL(5, 61, 18), // "onUpdateController"
QT_MOC_LITERAL(6, 80, 15), // "onUpdateForce1D"
QT_MOC_LITERAL(7, 96, 15), // "onUpdateForce2D"
QT_MOC_LITERAL(8, 112, 12), // "onUpdateTime"
QT_MOC_LITERAL(9, 125, 14), // "onSelectEntity"
QT_MOC_LITERAL(10, 140, 16), // "QTreeWidgetItem*"
QT_MOC_LITERAL(11, 157, 12), // "onActionExit"
QT_MOC_LITERAL(12, 170, 12) // "onActionSave"

    },
    "LevelEditor\0onUpdatePosition\0\0"
    "onUpdateHalfDim\0onUpdateRadius\0"
    "onUpdateController\0onUpdateForce1D\0"
    "onUpdateForce2D\0onUpdateTime\0"
    "onSelectEntity\0QTreeWidgetItem*\0"
    "onActionExit\0onActionSave"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_LevelEditor[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x0a /* Public */,
       3,    0,   65,    2, 0x0a /* Public */,
       4,    0,   66,    2, 0x0a /* Public */,
       5,    0,   67,    2, 0x0a /* Public */,
       6,    0,   68,    2, 0x0a /* Public */,
       7,    0,   69,    2, 0x0a /* Public */,
       8,    0,   70,    2, 0x0a /* Public */,
       9,    2,   71,    2, 0x0a /* Public */,
      11,    0,   76,    2, 0x0a /* Public */,
      12,    0,   77,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 10, QMetaType::Int,    2,    2,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void LevelEditor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        LevelEditor *_t = static_cast<LevelEditor *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onUpdatePosition(); break;
        case 1: _t->onUpdateHalfDim(); break;
        case 2: _t->onUpdateRadius(); break;
        case 3: _t->onUpdateController(); break;
        case 4: _t->onUpdateForce1D(); break;
        case 5: _t->onUpdateForce2D(); break;
        case 6: _t->onUpdateTime(); break;
        case 7: _t->onSelectEntity((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 8: _t->onActionExit(); break;
        case 9: _t->onActionSave(); break;
        default: ;
        }
    }
}

const QMetaObject LevelEditor::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_LevelEditor.data,
      qt_meta_data_LevelEditor,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *LevelEditor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LevelEditor::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_LevelEditor.stringdata0))
        return static_cast<void*>(const_cast< LevelEditor*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int LevelEditor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
