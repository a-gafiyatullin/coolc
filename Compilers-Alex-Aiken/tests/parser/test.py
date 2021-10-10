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
myparser = root_dir + '/bin/new-parser'

print(bcolors.BOLD + 'Test directory: ' + bcolors.ENDC + current_dir + '\n')

if not rebuild_parser and os.path.exists(myparser):
    print('new-parser already exists in bin directory. Skip building.\n')
else:
    os.chdir(root_dir + '/src/parser')
    if rebuild_with_asan:
        os.system('make parser-asan')
    elif rebuild_with_ubsan:
        os.system('make parser-ubsan')
    else:
        os.system('make parser')
    os.chdir(current_dir)
    print('')

print(bcolors.BOLD +'Reference lexer: ' + bcolors.ENDC + reference_lexer + '\n')
if not os.path.exists(reference_lexer):
    print("Can't find reference lexer!")
    
print(bcolors.BOLD +'Reference parser: ' + bcolors.ENDC + reference_parser + '\n')
if not os.path.exists(reference_parser):
    print("Can't find reference parser!")

test_num = 0

print(bcolors.HEADER + '-------------------- START TESTS! --------------------' + bcolors.ENDC)
for filename in os.listdir(current_dir + '/end-to-end'):
    test_num += 1
    if filename.endswith(".test"):
        result = str(test_num) + ') ' + bcolors.BOLD + filename + bcolors.ENDC
        
        myresult = subprocess.run([myparser, 'end-to-end/' + filename], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        reference_lexer_result = subprocess.Popen([reference_lexer, 'end-to-end/' + filename], stdout=subprocess.PIPE)
        reference_parser_result = subprocess.run([reference_parser], stdin=reference_lexer_result.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if (myresult.stdout == reference_parser_result.stdout or myresult.stdout in reference_parser_result.stderr) and myresult.stderr == b'':
            sys.stdout.write("%-55s %s\n" % (result, bcolors.OKGREEN + '[OK]' + bcolors.ENDC))
        else:
            sys.stdout.write("%-55s %s\n\n" % (result, bcolors.FAIL +  '[FAILED]' + bcolors.ENDC))
            print(bcolors.BOLD + 'new-parser-stdout:        ' + bcolors.ENDC, end='')
            print(myresult.stdout)
            print(bcolors.BOLD + 'new-parser-stderr:        ' + bcolors.ENDC, end='')
            print(myresult.stderr)
            print(bcolors.BOLD + 'reference-parser-stdout: ' + bcolors.ENDC, end='')
            print(reference_parser_result.stdout)
            print(bcolors.BOLD + 'reference-parser-stderr: ' + bcolors.ENDC, end='')
            print(reference_parser_result.stderr)
            break

print(bcolors.HEADER + '-------------------- END TESTS! --------------------' + bcolors.ENDC)