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

reference_parser = root_dir + '/bin/parser'
reference_lexer = root_dir + '/bin/lexer'
reference_semant = root_dir + '/bin/semant'
mysemant = root_dir + '/bin/new-semant'

print(bcolors.BOLD + 'Test directory: ' + bcolors.ENDC + current_dir + '\n')

if not rebuild_parser and os.path.exists(mysemant):
    print('new-semant already exists in bin directory. Skip building.\n')
else:
    os.chdir(root_dir + '/src/semant')
    if rebuild_with_asan:
        os.system('make semant-asan')
    elif rebuild_with_ubsan:
        os.system('make semant-ubsan')
    else:
        os.system('make semant')
    os.chdir(current_dir)
    print('')

print(bcolors.BOLD +'Reference lexer: ' + bcolors.ENDC + reference_lexer + '\n')
if not os.path.exists(reference_lexer):
    print("Can't find reference lexer!")
    
print(bcolors.BOLD +'Reference parser: ' + bcolors.ENDC + reference_parser + '\n')
if not os.path.exists(reference_parser):
    print("Can't find reference parser!")

print(bcolors.BOLD +'Reference semant: ' + bcolors.ENDC + reference_semant + '\n')
if not os.path.exists(reference_semant):
    print("Can't find reference semant!")

def do_tests_in_folder_with_ext(folder, ext, header, ignore_list):
    test_num = 0

    print(bcolors.HEADER + '-------------------- START ' + header + ' TESTS! --------------------' + bcolors.ENDC)
    for filename in os.listdir(folder):
        test_num += 1
        if filename.endswith(ext):
            result = str(test_num) + ') ' + bcolors.BOLD + filename + bcolors.ENDC

            myresult = subprocess.run([mysemant, folder + filename], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            reference_lexer_result = subprocess.Popen([reference_lexer, folder + filename], stdout=subprocess.PIPE)
            reference_parser_result = subprocess.Popen([reference_parser], stdin=reference_lexer_result.stdout, stdout=subprocess.PIPE)
            reference_semant_result = subprocess.run([reference_semant], stdin=reference_parser_result.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            if filename in ignore_list:
                if myresult.returncode == 0:
                    sys.stdout.write("%-55s %s\n" % (result, bcolors.OKGREEN + '[OK]' + bcolors.ENDC))
            elif (myresult.stdout == reference_semant_result.stdout or myresult.stdout in reference_semant_result.stderr) and myresult.stderr == b'':
                sys.stdout.write("%-55s %s\n" % (result, bcolors.OKGREEN + '[OK]' + bcolors.ENDC))
            else:
                sys.stdout.write("%-55s %s\n\n" % (result, bcolors.FAIL +  '[FAILED]' + bcolors.ENDC))
                print(bcolors.BOLD + 'new-semant-stdout:        ' + bcolors.ENDC, end='')
                print(myresult.stdout)
                print(bcolors.BOLD + 'new-semant-stderr:        ' + bcolors.ENDC, end='')
                print(myresult.stderr)
                print(bcolors.BOLD + 'reference-semant-stdout: ' + bcolors.ENDC, end='')
                print(reference_semant_result.stdout)
                print(bcolors.BOLD + 'reference-semant-stderr: ' + bcolors.ENDC, end='')
                print(reference_semant_result.stderr)
                break

    print(bcolors.HEADER + '-------------------- END TESTS! --------------------' + bcolors.ENDC)

do_tests_in_folder_with_ext(current_dir + '/end-to-end/', '.test', 'end-to-end', ["cycle.test"])
do_tests_in_folder_with_ext(root_dir + '/examples/', '.cl', 'examples', [])