/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * This file is part of OpenHydroQual.
 *
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@enviroinformatics.co for details.
 */

// CLM is a template defined entirely in LM.hpp, like CGA. This translation
// unit exists so the class is instantiated once for System in a fixed place
// instead of in whichever caller happens to be compiled first, which keeps
// the (substantial) instantiation out of every GUI object file.

#include "System.h"
#include "LM/LM.h"

template class CLM<System>;
