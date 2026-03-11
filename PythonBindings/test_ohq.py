import sys
sys.path.append('/home/arash/Projects/OpenHydroQual/PythonBindings/build/Desktop_Qt_6_8_2-Release/')

import openhydroqual_py as ohq

# Create system
system = ohq.System()

# Set paths
system.set_default_template_path('/home/arash/Projects/OpenHydroQual/resources/')
system.set_working_folder('/home/arash/Dropbox/Aquifolium/Groundwater_CEE491/')
system.set_silent(False)

# Load and run
system.create_from_script(
    '/home/arash/Dropbox/Aquifolium/Groundwater_CEE491/1DModel.ohq',
    '/home/arash/Projects/OpenHydroQual/resources/settings.json'
)

system.solve()
system.write_outputs()

print('Simulation complete')
