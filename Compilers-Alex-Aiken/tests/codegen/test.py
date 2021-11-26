import os
import subprocess
import sys

class bcolors:
    HEADER = '\033[95m'
    OKGREEN = '\033[92m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

rebuild_parser = False
rebuild_with_asan = False
rebuild_with_ubsan = False

if len(sys.argv) > 1:
    for key in sys.argv[1:]:
        if key == '-build':
            rebuild_parser = True
        if key == '-asan':
            rebuild_parser = True
            rebuild_with_asan = True
        if key == '-ubsan':
            rebuild_parser = True
            rebuild_with_ubsan = True

current_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = current_dir + '/../..'

spim = root_dir + '/bin/spim'
reference_codegen = root_dir + '/bin/coolc'
mycoolc = root_dir + '/bin/new-codegen'

print(bcolors.BOLD + 'Test directory: ' + bcolors.ENDC + current_dir + '\n')

if not rebuild_parser and os.path.exists(mycoolc):
    print('new-codegen already exists in bin directory. Skip building.\n')
else:
    os.chdir(root_dir + '/src/codegen/stack_machine/mips/spim')
    if rebuild_with_asan:
        os.system('make codegen-asan')
    elif rebuild_with_ubsan:
        os.system('make codegen-ubsan')
    else:
        os.system('make codegen')
    os.chdir(current_dir)
    print('')

print(bcolors.BOLD +'Reference codegen: ' + bcolors.ENDC + reference_codegen + '\n')
if not os.path.exists(reference_codegen):
    print("Can't find reference codegen!")

def do_tests_in_folder_with_ext(folder, ext, header):
    test_num = 0

    print(bcolors.HEADER + '-------------------- START ' + header + ' TESTS! --------------------' + bcolors.ENDC)
    for filename in os.listdir(folder):
        if filename.endswith(ext):
            test_num += 1
            result = str(test_num) + ') ' + bcolors.BOLD + filename + bcolors.ENDC

            result_file_name = folder + filename.replace(".cl", ".s")

            os.remove(result_file_name)
            subprocess.run([mycoolc, folder + filename])
            myspim = subprocess.run([spim, result_file_name], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            
            os.remove(result_file_name)
            subprocess.run([reference_codegen, "-g", folder + filename])
            ref_spim = subprocess.run([spim, result_file_name], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            if (myspim.stdout == ref_spim.stdout):
                sys.stdout.write("%-55s %s\n" % (result, bcolors.OKGREEN + '[OK]' + bcolors.ENDC))
            else:
                sys.stdout.write("%-55s %s\n\n" % (result, bcolors.FAIL +  '[FAILED]' + bcolors.ENDC))
                print(bcolors.BOLD + 'new-coolc-stdout:        ' + bcolors.ENDC, end='')
                print(myspim.stdout)
                print(bcolors.BOLD + 'reference-coolc-stdout: ' + bcolors.ENDC, end='')
                print(ref_spim.stdout)
                break

    print(bcolors.HEADER + '-------------------- END TESTS! --------------------' + bcolors.ENDC)

do_tests_in_folder_with_ext(current_dir + '/end-to-end/', '.cl', 'end-to-end')