/* Copyright (C) 2009-2010 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include <usbase/utilities.h>
#include "lactin_time.h"

namespace UniSim{

LactinTime::LactinTime(UniSim::Identifier name, QObject *parent)
	: Model(name, parent)
{
    setState("step", &step);
    setState("total", &total);
}

void LactinTime::initialize()
{
    setParameter("a", &a, 0.13);
    setParameter("b", &b, 42.);
    setParameter("c", &c, 8.);
    setParameter("d", &d, -0.1);
    weather = findOne<Model*>("weather");
}

void LactinTime::reset() {
    step = total = 0.;
}

void LactinTime::update()
{
    double T = weather->state("Tavg");
    step = exp(a*T) - exp(a*b - (b - T)/c) + d;
    if (step < 0.)
        step = 0.;
    total += step;
}

} //namespace
