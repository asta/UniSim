/* Copyright (C) 2009-2010 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#ifndef UniSim_UTILITIES_H
#define UniSim_UTILITIES_H

#include <cmath>
#include <cfloat>
#include <QDir>
#include <QList>
#include <QMap>
#include <QObject>
#include <QPluginLoader>
#include <QString>
#include <QStringList>
#include "exception.h"
#include "file_locations.h"
#include "identifier.h"
#include "object_pool.h"

#include <iostream>
using std::cout;

namespace UniSim {

const double PI = 3.14159265;
const double DEG_TO_RAD = 0.017453293;
const double MAX_EXP = -log(DBL_EPSILON);
const double MIN_EXP = -log(1. - DBL_EPSILON);

class Model;

//! @name Versioning
//@{
QString version();
bool isDeveloperVersion();

//@}

//! @name Navigation
//@{
void setSimulationObjectFromDescendent(QObject *object);

template <class T> T seekOneChild(QString name, QObject *parent);
template <class T> QList<T> seekChildren(QString name, QObject *parent);

template <class T> T seekOneDescendant(QString name, QObject *root);
template <class T> QList<T> seekManyDescendants(QString name, QObject *root);

template <class T> T seekOneAscendant(QString name, QObject *child);

template <class T> QList<T> filterByName(QString name, const QList<QObject*> &candidates);
//@}

//! \cond
class SimulationObject {
    private:
    static QObject* simulation;
    friend void setSimulationObject(QObject *simulation);
    friend void setSimulationObjectFromDescendent(QObject *descendent);
    friend QObject* simulationObject();
};
void setSimulationObject(QObject *simulation);
void setSimulationObjectFromDescendent(QObject *descendent);
QObject* simulationObject();
//! \endcond

//! @name Mathematics
//@{
double interpolate(const QMap<int, double> xy, int x);
double pow0(double x, double c);
double negExp(double x);
double GBFuncResp(double demand, double apparency);
int toDayOfYear(int day, int month);
//@}

//! @name String handling
//@{
QStringList decodeSimpleList(QString parenthesizedList, QString errorContext = QString());
void splitAtNamespace(QString s, QString *namespacePart, QString *ownNamePart);
//@}

//! @name PlugIn handling
//@{
template <class TPlugin>
void lookupPlugIns(QString makerId, QMap<Identifier, TPlugin*> *makers);
//@}

//! @name Testing
//@{
void writeObjectTree(QObject *root, int level = 0);
void writeStandardTestFile(QString filePath);
//@}

// ========================
// Template implementations

//! Finds any number of objects
/*!

  The path identifying the object(s) can be atomic ("elephant"),
  or, it can specify a path relative to the root ("mammals/elephant").
  Jokers are also allowed ("*", "mammals/ *", "* /leaves)
*/
template <class T> QList<T> seekDescendants(QString name, QObject *root) {
    QObject *useRoot = root ? root : simulationObject();
    QList<QObject*> candidates = useRoot->findChildren<QObject*>();
    if (!root)
        candidates.prepend(useRoot);
    return filterByName<T>(name, candidates);
}

//! Finds exactly one object
/*!
  Works like find but throws an Exception if not exactly one object is found
*/
 template <class T> T seekOneDescendant(QString name, QObject *root) {
    QObject *useRoot = root ? root : simulationObject();
    QList<T> matches = seekDescendants<T>(name, useRoot);
    if (matches.size() == 0)
        throw Exception("'" + useRoot->objectName() +
                        "' has no descendant called '" + name + "'");
    else if (matches.size() > 1)
        throw Exception("'" + useRoot->objectName() +
                        "' has more than one descendant called '" + name + "'");
    return matches[0];
}

 //! Finds any number of children
/*!
    Works like find but searches only the children of parent. Grandchildren and further descendant are not sought.
    (unlike Qt's findChildren which searches recursively and not only children).

    If parent is null then the root is the assumed parent,
*/

 template <class T> QList<T> seekChildren(QString name, QObject *parent) {
    QObject *useParent = parent ? parent : simulationObject();
    QList<QObject*> candidates = useParent->children();
    return filterByName<T>(name, candidates);
}

//! Finds exactly one child
/*!
  Works like findChildren but throws an Exception if not exactly one object is found
*/
template <class T> T seekOneChild(QString name, QObject *parent) {
    QObject *useParent = parent ? parent : simulationObject();
    QList<T> matches = seekChildren<T>(name, useParent);
    if (matches.size() == 0)
        throw Exception("'" + useParent->objectName() +
                        "' has no child called '" + name + "'");
    else if (matches.size() > 1)
        throw Exception("'" + useParent->objectName() +
                        "' has more than one child called '" + name + "'");
    return matches[0];
}

//! Finds an ascendant of child or throws an exception
/*!
    Finds one ascendant matching name and type, or throws an Exception
*/
template <class T> T seekOneAscendant(QString name, QObject *child) {
    if (!child)
        throw Exception("findAscendant called with null pointer");

    QList<QObject*> candidates;
    QObject *p = child->parent();
    while (p) {
        candidates.append(p);
        p = p->parent();
    }

    QList<T> ascendants = filterByName<T>(name, candidates);
    if (ascendants.size() == 0)
        throw Exception("'" + child->objectName() +"' has no ascendants called '" +
                        name + "', or it is not of the expected type");
    if (ascendants.size() > 1)
        throw Exception("'" + child->objectName() +"' has more than one ascendant called '" +
                        name);
    return ascendants.at(0);
}

//! Selects objects in candidate list matching name
template <class T> QList<T> filterByName(QString name, const QList<QObject*> &candidates) {
    QList<T> result;
    QStringList names = Identifier(name).key().split("/");
    if (!names.isEmpty() && names.first().isEmpty())
        names.removeFirst();
    if (!names.isEmpty() && names.last().isEmpty())
        names.removeLast();
    if (names.isEmpty())
        return result;

    for (int ca = 0; ca < candidates.size(); ++ca) {
        int i = names.size()-1;
        QObject *candidate, *p;
        candidate = p = candidates.at(ca);
        while (i > 0 && p->parent() && (p->objectName() == names[i] || names[i] == "*")) {
            --i;
            p = p->parent();
        }
        if (names[i].isEmpty())
            throw Exception("Composite component name contains an empty name: " + name);
        bool isT = dynamic_cast<T>(candidate) != 0;
        bool pathOk  =	i == 0 && (p->objectName() == names[0] || names[0] == "*");
        if (isT && pathOk) {
            result.append(dynamic_cast<T>(candidate));
        }
    }
    return result;
}

template <class TPlugin>
void lookupPlugIns(QString makerId, QMap<Identifier, TPlugin*> *makers) {
    bool keepLooking = true;
    do {
        QDir dir = FileLocations::location(FileLocations::Plugins);
        foreach (QString filename, dir.entryList(QDir::Files)) {

            QPluginLoader loader(dir.absoluteFilePath(filename));

            TPlugin *plugin = qobject_cast<TPlugin*>(loader.instance());
            if (plugin) {
                plugin->useObjectPool(objectPool());
				QList<Identifier> classes = plugin->supportedClasses().keys();
                foreach (Identifier id, classes) {
                    (*makers)[id] = plugin;
                    Identifier idWithNamespace = plugin->pluginName().label() + "::" + id.label();
                    (*makers)[idWithNamespace] = plugin;

                }
            }
        }

        if (makers->size() > 0) {
            keepLooking = false;
        }
        else {
            QString msg = "Found no plugins for: " + makerId + " in: " + dir.absolutePath();
            if (!dir.exists())
                msg += ".\nThe folder does not exist.";
            keepLooking = FileLocations::lookup(FileLocations::Plugins, msg);
        }
    } while (keepLooking);
}

} //namespace

#endif
