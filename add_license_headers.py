import os

LICENSE_HEADER = '''\
/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
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
'''

def has_license(content):
    return 'OpenHydroQual' in content and 'GNU Affero General Public License' in content

def add_license_to_file(filepath):
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
    except UnicodeDecodeError:
        print(f"Skipping (encoding issue): {filepath}")
        return

    if has_license(content):
        print(f'Skipping (already licensed): {filepath}')
        return

    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(LICENSE_HEADER + '\n\n' + content)
    print(f'✅ Added license to: {filepath}')

def scan_directory(root_dir):
    for subdir, _, files in os.walk(root_dir):
        for file in files:
            if file.endswith('.hpp'):
                filepath = os.path.join(subdir, file)
                add_license_to_file(filepath)

if __name__ == "__main__":
    project_root = '.'  # or replace with your actual path
    scan_directory(project_root)

