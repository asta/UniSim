/* Copyright (C) 2009-2010 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#ifndef UniSim_FILE_LOCATIONS_FORGIVING_H
#define UniSim_FILE_LOCATIONS_FORGIVING_H

#include <usbase/file_locations.h>

class FileLocationsForgiving : public UniSim::FileLocations {
public:
    FileLocationsForgiving();
private:
	QDir locationImpl(FileType fileType);
};

#endif