# check for depedencies files
import os
import sys

from SCons.Script import Import


Import("env")



def create_dummy_file(file_path:str):
    os.makedirs(os.path.dirname(file_path), exist_ok=True)  # Ensure the directory exists
    with open(file_path, 'w') as f:
        f.write("dummy\n")  # Optional: You can write default content
def check_deps(prj_dir:str):
    # Define the list of files to check
    deps = [
        {
            'name':'BMV080 SDK',
            'files':[
                'deps/bosch-sensortec/BMV080-SDK/api/lib/xtensa_esp32s3/xtensa_esp32s3_elf_gcc/release/lib_bmv080.a',
                'deps/bosch-sensortec/BMV080-SDK/api/lib/xtensa_esp32s3/xtensa_esp32s3_elf_gcc/release/lib_postProcessor.a',
                'deps/bosch-sensortec/BMV080-SDK/api/inc/bmv080.h',
                'deps/bosch-sensortec/BMV080-SDK/api/inc/bmv080_defs.h',
                'deps/bosch-sensortec/BMV080-SDK/api_examples/_common/inc/bmv080_example.h'
                ],

            'wheretoget':'https://www.bosch-sensortec.com/software-tools/double-opt-in-forms/sdk-v11-0-0.html'
        },
        {
            'name':'BSEC3 Library',
            'files': [
                'deps/bosch-sensortec/bsec_library_v3.x/algo/bsec_IAQ/bin/esp/esp32_s3/libalgobsec.a',
                'deps/bosch-sensortec/bsec_library_v3.x/algo/bsec_IAQ/inc/bsec_datatypes.h',
                'deps/bosch-sensortec/bsec_library_v3.x/algo/bsec_IAQ/inc/bsec_interface.h',
                'deps/bosch-sensortec/bsec_library_v3.x/algo/bsec_IAQ/config/bme690/bme690_iaq_33v_3s_4d/bsec_iaq.h',
                'deps/bosch-sensortec/bsec_library_v3.x/algo/bsec_IAQ/config/bme690/bme690_iaq_33v_3s_4d/bsec_iaq.c',
                ],
            'wheretoget':'https://www.bosch-sensortec.com/software-tools/double-opt-in-forms/bsec-software-3-1-0-0-form-1.html'
        },
    ]

    # Check if each file exists
    for dep in deps:
        name = dep['name']
        where = dep['wheretoget']
        files = dep['files']

        for file in files:
            file_path_amend = os.path.join(prj_dir, '../' + file)
            file_path_full = os.path.abspath(file_path_amend)
            if not os.path.exists(file_path_full):
                #create_dummy_file(file_path_amend)
                print('*' * 160)
                print(f"Error: Missing file '{file_path_full}' from the {name} which can be get from:<{where}>")
                print(f"After the zip file is downloaded, extract it to the right location so that the above file could be located")
                print('*' * 160)
                sys.exit(1)  # Abort the build if any file is missing



def before_build(source, target, env):
    check_deps(env.subst("PROJECT_DIR"))

print("Current CLI targets", COMMAND_LINE_TARGETS)
print("Current Build targets", BUILD_TARGETS)
before_build(None, None, env)

# Register the function to be called before the build process
#env.AddPreAction("build", before_build)
