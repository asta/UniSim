#include <iostream>
#include <QSettings>
#include <usbase/exception.h>
#include <usbase/identifier.h>
#include <usbase/file_locations.h>
#include <usbase/output.h>
#include <usbase/utilities.h>
#include <standard_controllers/simple_controller.h>
#include <standard_models/anonymous_model.h>
#include <usengine/simulation_maker.h>
#include "../simulation.h"
#include "trickle_box.h"
#include "trickle_sequence.h"
#include "test_simulation.h"


using namespace UniSim;
	
void TestSimulation::initTestCase()
{	
    QDir dir = FileLocations::location(FileLocations::Temporary);
    QString filePath = dir.absolutePath() + "/test.xml";
    writeStandardTestFile(filePath);

	_simulation = 0;
	SimulationMaker maker;
	bool modelOk = false;
	try {
        _simulation = maker.parse(filePath);
		modelOk = true;
	}
	catch (const UniSim::Exception &ex) {
		delete _simulation;
		_simulation = 0;
		QWARN(qPrintable(ex.message()));
		QVERIFY(false);
	}
}

void TestSimulation::init() {
    setSimulationObject(_simulation);
}

void TestSimulation::cleanupTestCase()
{
	delete _simulation;
	_simulation = 0;
}

void TestSimulation::testInitialize()
{
	QVERIFY(_simulation);
	//writeObjectTree(_simulation);
	QCOMPARE(_simulation->state(), Simulation::Initialized);
}

void TestSimulation::testFindModels() 
{
    Simulation *sim = new Simulation("apple-tree", "1.0");
    setSimulationObject(sim);
	new SimpleController("controller", sim);
	
	AnonymousModel *butterfly, *mite;
	
	butterfly = new AnonymousModel("butterfly", sim);
	new AnonymousModel("egg", butterfly);
	new AnonymousModel("larva", butterfly);
	new AnonymousModel("pupa", butterfly);
	new AnonymousModel("adult", butterfly);
	
	mite = new AnonymousModel("mite", sim);
	new AnonymousModel("egg", mite);
	new AnonymousModel("nymph", mite);
	new AnonymousModel("adult", mite);
	
    sim->initialize(Identifiers() << "butterfly" << "mite");
	QCOMPARE(_simulation->state(), Simulation::Initialized);

    QList<Model*> models;
    models = find<Model*>("adult");
	QCOMPARE(models.size(), 2);
	QVERIFY(models[0]->parent());
	QCOMPARE(models[0]->parent()->objectName(), QString("butterfly"));
	QVERIFY(models[1]->parent());
	QCOMPARE(models[1]->parent()->objectName(), QString("mite"));
	
    models = find<Model*>("mite/egg");
	QCOMPARE(models.size(), 1);
	QCOMPARE(models[0]->objectName(), QString("egg"));
	QVERIFY(models[0]->parent());
	QCOMPARE(models[0]->parent()->objectName(), QString("mite"));
	
    QList<Model*> same;
    same = find<Model*>("/mite/egg");
	QCOMPARE(same.size(), 1);
	QCOMPARE(models[0], same[0]);
	
    models = find<Model*>("/mite");
	QCOMPARE(same.size(), 1);
	QCOMPARE(models[0]->objectName(), QString("mite"));

    models = find<Model*>("/");
	QCOMPARE(models.size(), 0);
	
    models = find<Model*>("");
	QCOMPARE(models.size(), 0);

    models = find<Model*>("/ghost/egg");
	QCOMPARE(models.size(), 0);

    models = find<Model*>("ghost/egg");
	QCOMPARE(models.size(), 0);

	// Jokers
    models = find<Model*>("butterfly/*");
	QCOMPARE(models.size(), 4);
	QCOMPARE(models[0]->objectName(), QString("egg"));
	QCOMPARE(models[3]->objectName(), QString("adult"));
	
    models = find<Model*>("/butterfly/*");
	QCOMPARE(models.size(), 4);
	QCOMPARE(models[0]->objectName(), QString("egg"));
	QCOMPARE(models[3]->objectName(), QString("adult"));
		
    models = UniSim::findChildren<Model*>("*");
	QCOMPARE(models.size(), 2);
	QCOMPARE(models[0]->objectName(), QString("butterfly"));
	QCOMPARE(models[1]->objectName(), QString("mite"));
		
	delete sim;
}

void TestSimulation::testFindOutputs()
{
    QList<Output*> outputs;
    outputs = find<Output*>("butterflyTable");
    QCOMPARE(outputs.size(), 1);
    QVERIFY(Identifier("butterflyTable").equals(outputs[0]->objectName()));
}

void TestSimulation::testExecute()
{
    QList<Model*> search;
    search = find<Model*>("/butterfly/egg");
    QCOMPARE(search.size(), 1);
    search[0]->setInput("input", 15);

    search = find<Model*>("/wasp/egg");
    QCOMPARE(search.size(), 1);
    search[0]->setInput("input", 80);

	try {
		_simulation->execute();
	}
	catch (const UniSim::Exception &ex) {
		QWARN(qPrintable(ex.message()));
		QVERIFY(false);
	}
}