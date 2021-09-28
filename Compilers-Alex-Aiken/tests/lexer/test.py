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

rebuild_lexer = False
rebuild_with_asan = False
rebuild_with_ubsan = False

if len(sys.argv) > 1:
    for key in sys.argv[1:]:
        if key == '-build':
            rebuild_lexer = True
        if key == '-asan':
            rebuild_lexer = True
            rebuild_with_asan = True
        if key == '-ubsan':
            rebuild_lexer = True
            rebuild_with_ubsan = True

current_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = current_dir + '/../..'

reference_lexer = root_dir + '/bin/lexer'
mylexer = root_dir + '/bin/new-lexer'

print(bcolors.BOLD + 'Test directory: ' + bcolors.ENDC + current_dir + '\n')

if not rebuild_lexer and os.path.exists(mylexer):
    print('new-lexer already exists in bin directory. Skip building.\n')
else:
    os.chdir(root_dir + '/src/lexer')
    if rebuild_with_asan:
        os.system('make lexer-asan')
    elif rebuild_with_ubsan:
        os.system('make lexer-ubsan')
    else:
        os.system('make lexer')
    os.chdir(current_dir)
    print('')

print(bcolors.BOLD +'Reference lexer: ' + bcolors.ENDC + reference_lexer + '\n')
if not os.path.exists(reference_lexer):
    print("Can't find reference lexer!")

test_num = 0

print(bcolors.HEADER + '-------------------- START TESTS! --------------------' + bcolors.ENDC)
for filename in os.listdir(current_dir + '/end-to-end'):
    test_num += 1
    if filename.endswith(".cool"):
        result = str(test_num) + ') ' + bcolors.BOLD + filename + bcolors.ENDC
        
        myresult = subprocess.run([mylexer, 'end-to-end/' + filename], stdout=subprocess.PIPE)
        reference_result = subprocess.run([reference_lexer, 'end-to-end/' + filename], stdout=subprocess.PIPE)
        if myresult.stdout == reference_result.stdout:
            sys.stdout.write("%-55s %s\n" % (result, bcolors.OKGREEN + '[OK]' + bcolors.ENDC))
        else:
            sys.stdout.write("%-55s %s\n\n" % (result, bcolors.FAIL +  '[FAILED]' + bcolors.ENDC))
            print(bcolors.BOLD + 'new-lexer:        ' + bcolors.ENDC, end='')
            print(myresult.stdout)
            print(bcolors.BOLD + 'reference-lexer: ' + bcolors.ENDC, end='')
            print(reference_result.stdout)
            break

print(bcolors.HEADER + '-------------------- END TESTS! --------------------' + bcolors.ENDC)