/* Copyright (C) 2009-2010 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#ifndef UniSim_UTILITIES_H
#define UniSim_UTILITIES_H

#include <cmath>
#include <cfloat>
#include <QObject>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include "exception.h"
#include "identifier.h"

#include <iostream>
using std::cout;

namespace UniSim {

const double PI = 3.14159265;
const double DEG_TO_RAD = 0.017453293;
const double MAX_EXP = -log(DBL_EPSILON);
const double MIN_EXP = -log(1. - DBL_EPSILON);

class Model;

//! @name Navigation
//@{
void setSimulationObjectFromDescendent(QObject *object);

template <class T> QList<T> find(QString path, QObject *root = 0);
template <class T> T findOne(QString path, QObject *root = 0);
template <class T> QList<T> findChildren(QString name, QObject *parent = 0);
template <class T> T findChild(QString name, QObject *parent = 0);
template <class T> T findAscendant(QString name, QObject *child);

template <class T> QList<T> find(QString path, const QList<QObject*> &candidates);
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

//! @name Testing
//@{
void writeObjectTree(QObject *root, int level = 0);
void writeStandardTestFile(QString filePath);
//@}

// ========================
// Template implementations

//! Finds any number of objects
/*!

  The path identifying the object(s) can be atomic:
  <ul>
    <li>QList<Model*> weeds = find<Model*>("daisy");
        - finds globally all models named "daisy"
    <li>QList<Model*> weeds = find<Model*>("daisy", field);
        - finds, among descendants of 'field', all models named "daisy"
    <li>QList<Weed*> weeds = find<Weed*>("*");
        - finds globally all models of class 'Weed'
    <li>QList<Weed*> weeds = find<Weed*>("*", field);
        -finds, among descendants of 'field', all models of class 'Weed'
    <li>QList<Weed*> weeds = find<Weed*>("daisy");
        - finds globally all models of class 'Weed' that are named "daisy"
    <li>QList<Weed*> weeds = find<Weed*>("daisy", field);
        -finds, among descendants of 'field', all models of class 'Weed' that are named "daisy"
   </ul>

   Or, it can specify a path relative to the root (any initial '/' is ignored):
   <ul>
    <li>QList<Model*> weeds = find<Model*>("daisy/leaves");
        - finds globally all models named "leaves" that are children of an object named 'daisy'
    <li>QList<Model*> weeds = find<Model*>("daisy/ * /cells");
        -finds globally all models named "cells" that are grandchildren of an object named 'daisy'
    <li>QList<Model*> weeds = find<Model*>("daisy/ *");
        - finds globally all models  that are children of an object named 'daisy'
    <li>QList<Model*> weeds = find<Model*>("daisy/leaves", field);
        - finds, among descendants of 'field', all models named "leaves" that are children of an object named 'daisy'
    <li>QList<Model*> weeds = find<Model*>("daisy/ * /cells", field);
        - finds, among descendants of 'field', all models named "cells" that are grandchildren of an object named 'daisy'
    <li>QList<Model*> weeds = find<Model*>("daisy/ *", field);
        - finds, among descendants of 'field', all models  that are children of an object named 'daisy'

*/
template <class T> QList<T> find(QString path, QObject *root) {
    //cout << "***find " << qPrintable(path) << "\n";
    QObject *useRoot = root ? root : simulationObject();
    QList<QObject*> candidates = useRoot->findChildren<QObject*>();
    if (!root)
        candidates.prepend(useRoot);
    return find<T>(path, candidates);
}

//! Finds exactly one object
/*!
  Works like find but throws an Exception if not exactly one object is found
*/
 template <class T> T findOne(QString path, QObject *root) {
    QObject *useRoot = root ? root : simulationObject();
    QList<T> matches = find<T>(path, useRoot);
    if (matches.size() == 0)
        throw Exception("'" + useRoot->objectName() + "' has no descendant called '" + path + "'");
    else if (matches.size() > 1)
        throw Exception("'" + useRoot->objectName() + "' has more than one descendant called '" + path + "'");
    return matches[0];
}

 //! Finds any number of children
/*!
    Works like find but searches only the children of parent. Grandchildren and further descendant are not sought.
    (unlike Qt's findChildren which searches recursively and not only children).

    If parent is null then the root is the assumed parent,
*/

 template <class T> QList<T> findChildren(QString name, QObject *parent) {
    QObject *useParent = parent ? parent : simulationObject();
    QList<QObject*> candidates = useParent->children();
    return find<T>(name, candidates);
}

//! Finds exactly one child
/*!
  Works like findChildren but throws an Exception if not exactly one object is found
*/
template <class T> T findChild(QString name, QObject *parent) {
    QObject *useParent = parent ? parent : simulationObject();
    QList<T> matches = findChildren<T>(name, useParent);
    if (matches.size() == 0)
        throw Exception("'" + useParent->objectName() + "' has no child called '" + name + "'");
    else if (matches.size() > 1)
        throw Exception("'" + useParent->objectName() + "' has more than one child called '" + name + "'");
    return matches[0];
}

//! Finds an ascendant of child or throws an exception
/*!
    Finds the most proximate ascendant matching name and type
*/
template <class T> T findAscendant(QString name, QObject *child) {
    if (!child)
        throw Exception("findAscendant called with null pointer");

    QList<QObject*> candidates;
    QObject *p = child->parent();
    while (p) {
        candidates.append(p);
        p = p->parent();
    }

    QList<T> ascendants = find<T>(name, candidates);
    if (ascendants.size() == 0)
        throw Exception("'" + child->objectName() +"' has no ascendants called '" + name + "', "
                        " or it is not of the expected type");
    return ascendants.at(0);
}

//! Finds any number of objects in list of candidates
template <class T> QList<T> find(QString path, const QList<QObject*> &candidates) {
    //cout << "-->find " << candidates.size();
    QList<T> result;
    QStringList names = Identifier(path).key().split("/");
    if (!names.isEmpty() && names.first().isEmpty())
        names.removeFirst();
    if (!names.isEmpty() && names.last().isEmpty())
        names.removeLast();
    if (names.isEmpty())
        return result;
    //cout << "-->removed being/end slash " << candidates.size() << "\n";

    for (int ca = 0; ca < candidates.size(); ++ca) {
        int i = names.size()-1;
        QObject *candidate, *p;
        candidate = p = candidates.at(ca);
        //cout << qPrintable(p->objectName() + "==" + names[i]) << "\n";
        while (i > 0 && p->parent() && (p->objectName() == names[i] || names[i] == "*")) {
            --i;
            p = p->parent();
            //cout << qPrintable("--> " + p->objectName() + "==" + names[i]) << "\n";
        }
        if (names[i].isEmpty())
            throw Exception("Path to component contains an empty name: " + path);
        bool isT = dynamic_cast<T>(candidate) != 0;
        bool pathOk  =	i == 0 && (p->objectName() == names[0] || names[0] == "*");
        //cout << "==> " << (isT ? "OK type" : "wrong type") << " " << (pathOk ? "OK path" : "wrong path") << "\n";
        if (isT && pathOk) {
            result.append(dynamic_cast<T>(candidate));
            //cout << "APPEND\n";
        }
    }
    return result;
}

} //namespace

#endif