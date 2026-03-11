import sys
sys.path.append('/home/arash/Projects/OpenHydroQual/PythonBindings/build/Desktop_Qt_6_8_2-Release/')

import openhydroqual_py as ohq

RESOURCES = '/home/arash/Projects/OpenHydroQual/resources/'
WORKDIR   = '/home/arash/Dropbox/Aquifolium/Groundwater_CEE491/'

system = ohq.System()
system.set_default_template_path(RESOURCES)
system.set_working_folder(WORKDIR)
system.set_silent(False)

# Load templates
system.load_template(RESOURCES + 'main_components.json')
system.add_template(RESOURCES + 'unconfined_groundwater.json')

# Simulation settings
system.set_system_value('simulation_end_time',   '90')
system.set_system_value('simulation_start_time', '0')
system.set_system_value('initial_time_step',     '0.01')
system.set_system_value('write_interval',        '100')

system2 = ohq.System()
system2.set_default_template_path('/home/arash/Projects/OpenHydroQual/resources/')
system2.set_working_folder('/home/arash/Dropbox/Aquifolium/Groundwater_CEE491/')
system2.create_from_script(
    '/home/arash/Dropbox/Aquifolium/Groundwater_CEE491/1DModel.ohq',
    '/home/arash/Projects/OpenHydroQual/resources/settings.json'
)
system2.save_to_script('/home/arash/Dropbox/Aquifolium/Groundwater_CEE491/saved_from_script.ohq')
print('Saved')

# Create groundwater cells
n = 10
for i in range(1, n + 1):
    head = 10.0 - (i - 1) * (10.0 - 9.0) / n
    system.create_block(
        name=f'cell({i}:1)',
        type='Unconfined Groundwater cell',
        properties={
            'area':                 '5000',
            'base_elevation':       '0',
            'hydraulic_conductivity': '5',
            'piezometric_head':     str(round(head, 5)),
            'porosity':             '0.3',
            'recharge_per_area':    WORKDIR + 'Recharge.csv',
            'specific_yield':       '0.2',
        }
    )

# Fixed head boundaries
system.create_block('fixed_head_left',  'fixed_head',
                    {'head': '10', 'Storage': '100000'})
system.create_block('fixed_head_right', 'fixed_head',
                    {'head': '9',  'Storage': '100000'})

# Internal links
for i in range(1, n):
    system.create_link(
        name=f'link({i}:{i+1})',
        type='UC_groundwater_link',
        from_block=f'cell({i}:1)',
        to_block=f'cell({i+1}:1)',
        properties={'length': '50', 'width': '100'}
    )

# Boundary links
system.create_link('link_left',  'unconfined_groundwater_to_fixedhead',
                   'cell(1:1)', 'fixed_head_left',
                   {'length': '50', 'width': '100'})
system.create_link('link_right', 'unconfined_groundwater_to_fixedhead',
                   f'cell({n}:1)', 'fixed_head_right',
                   {'length': '50', 'width': '100'})



print("=== link(1:2) BEFORE finalize ===")
print(system.get_link_json('link(1:2)'))

system.finalize()

print("=== link(1:2) AFTER finalize ===")
print(system.get_link_json('link(1:2)'))

system.solve()
system.write_outputs()

system.save_to_script(
    WORKDIR + 'model_saved.ohq',
    '',
    [RESOURCES + 'main_components.json',
     RESOURCES + 'unconfined_groundwater.json']
)

# Save as .json — full model state
system.save_to_json(
    '/home/arash/Dropbox/Aquifolium/Groundwater_CEE491/model_saved.json',
    addedtemplates=['main_components.json', 'unconfined_groundwater.json']
)

# Save .json with all variables and calculated values
system.save_to_json(
    '/home/arash/Dropbox/Aquifolium/Groundwater_CEE491/model_full.json',
    addedtemplates=['main_components.json', 'unconfined_groundwater.json'],
    allvariables=True,
    calculatevalue=True
)

print('Done')