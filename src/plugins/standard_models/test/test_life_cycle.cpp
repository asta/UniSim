#include <limits>
#include <cmath>
#include <iostream>
#include <QFile>
#include <QObject>
#include <usbase/model.h>
#include <usbase/utilities.h>
#include <usengine/model_maker.h>
#include "test_life_cycle.h"

using namespace UniSim;

void TestLifeCycle::initTestCase()
{
}

void TestLifeCycle::cleanupTestCase()
{
}

void TestLifeCycle::testUpdate()
{
	Model *weed = ModelMaker::create("LifeCycle", "biannual");
	QVERIFY(weed);
	
	Model *seedling = ModelMaker::create("LifeStage", "seedling", weed);
	QVERIFY(seedling);
	Model *juvenile = ModelMaker::create("LifeStage", "juvenile", weed);
	QVERIFY(juvenile);
	Model *mature = ModelMaker::create("LifeStage", "mature", weed);
	QVERIFY(mature);
	QCOMPARE(weed->children().size(), 3);

    weed->deepInitialize();
	
	seedling->changeParameter("k", 30);	
	seedling->changeParameter("duration", 50.);	
	
	juvenile->changeParameter("k", 20);	
	juvenile->changeParameter("duration", 200.);	
	
	mature->changeParameter("k", 10);	
	mature->changeParameter("duration", 1550.);	
	
    weed->deepReset();
	
    Model *first = UniSim::findChild<Model*>("seedling", weed);
    Model *last = UniSim::findChild<Model*>("mature", weed);
    QVERIFY(first);
    QVERIFY(last);

    Model *none;
    bool found;
    try {
        none = weed->findChild<Model*>("no way");
        found = true;
    }
    catch (Exception &ex) {
        found = false;
    }
    QVERIFY(!found);
	
	double myInput = 1000;
	first->setInput("input", myInput);
	
    static double EPS = std::min(myInput*1000*std::numeric_limits<double>::epsilon(),  1e-6);

	for (int i = 0; i < 10000; ++i) {
		weed->update();
	}

	QVERIFY2((fabs(first->state("inputTotal") - myInput) < EPS), 
			qPrintable("Expected: "+QString::number(myInput) 
			+ " Got: "+QString::number(first->state("inputTotal"))  
			+ " Diff: "+QString::number(fabs(first->state("inputTotal") - myInput)) 
			+ " > "+QString::number(EPS) 
			));
	QVERIFY2((fabs(first->state("outputTotal") - myInput) < EPS),
			qPrintable("Expected: "+QString::number(myInput) 
			+ " Got: "+QString::number(first->state("outputTotal")) 
			+ " Diff: "+QString::number(fabs(first->state("outputTotal") - myInput)) 
			+ " > "+QString::number(EPS) 
			));
	QVERIFY2((fabs(last->state("outputTotal") - myInput) < EPS),
			qPrintable("Expected: "+QString::number(myInput) 
			+ " Got: "+QString::number(last->state("outputTotal"))  
			+ " Diff: "+QString::number(fabs(last->state("outputTotal") - myInput))
			+ " > "+QString::number(EPS) 
			));

    weed->deepReset();
	first->setInput("input", myInput);
	for (int i = 0; i < 10000; ++i) {
        weed->deepUpdate();
	}

	QVERIFY2((fabs(first->state("inputTotal") - myInput) < EPS), 
			qPrintable("Expected: "+QString::number(myInput) 
			+ " Got: "+QString::number(first->state("inputTotal"))  
			+ " Diff: "+QString::number(fabs(first->state("inputTotal") - myInput)) 
			+ " > "+QString::number(EPS) 
			));
	QVERIFY2((fabs(first->state("outputTotal") - myInput) < EPS),
			qPrintable("Expected: "+QString::number(myInput) 
			+ " Got: "+QString::number(first->state("outputTotal")) 
			+ " Diff: "+QString::number(fabs(first->state("outputTotal") - myInput)) 
			+ " > "+QString::number(EPS) 
			));
	QVERIFY2((fabs(last->state("outputTotal") - myInput) < EPS),
			qPrintable("Expected: "+QString::number(myInput) 
			+ " Got: "+QString::number(last->state("outputTotal"))  
			+ " Diff: "+QString::number(fabs(last->state("outputTotal") - myInput))
			+ " > "+QString::number(EPS) 
			));


	delete weed;
}
 