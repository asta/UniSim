#include <usbase/integrator.h>
#include <usbase/identifier.h>
#include <usbase/utilities.h>
#include <usengine/simulation.h>
#include <standard_integrators/time_step_limited.h>
#include "test_simulation_trickle.h"
#include "trickle_box.h"
#include "trickle_sequence.h"

using namespace UniSim;

void TestSimulationTrickle::initTestCase() {
    simulation = new Simulation("trickles", "1.0");
    setSimulationObject(simulation);
    integrator = new TimeStepLimited("integrator", simulation);

    TrickleSequence *seq;
    seq = new TrickleSequence("sequence", simulation);
    boxes[0] = new TrickleBox("box0", seq);
    boxes[1] = new TrickleBox("box1", seq);
    boxes[2] = new TrickleBox("box2", seq);

    simulation->initialize(Identifiers() << "sequence");
    seq->deepReset();
}

void TestSimulationTrickle::cleanupTestCase() {
    delete simulation;
}


void TestSimulationTrickle::testExecute() {
    executeAndTest(0, 0, 0, 0);
    /*executeAndTest(1, 0, 0, 0);
    executeAndTest(5, 0, 0, 0);
    executeAndTest(6, 0, 0, 0);
    executeAndTest(14, 0, 0, 0);
    executeAndTest(15, 0, 0, 0);
    executeAndTest(20, 0, 0, 0);*/
}

void TestSimulationTrickle::executeAndTest(int steps, int check0, int check1, int check2) {
    integrator->changeParameter("maxSteps", steps);
    simulation->execute();
    textBox(0, check0);
    textBox(1, check1);
    textBox(2, check2);
}

void TestSimulationTrickle::textBox(int boxNumber, int contents) {
    QString name = "box"+QString::number(boxNumber);
    QList<Model*> models = UniSim::seekDescendants<Model*>(name, 0);
    QCOMPARE(models.size(), 1);

    QVERIFY(models[0]->pullVariablePtr("contents") != 0);
    QVERIFY(fabs(models[0]->pullVariable("contents") - double(contents)) < 1e-6);

    QVERIFY(boxes[boxNumber]->pullVariablePtr("contents") != 0);
    QVERIFY(fabs(boxes[boxNumber]->pullVariable("contents") - double(contents)) < 1e-6);
}

