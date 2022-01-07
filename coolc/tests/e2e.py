import os
import subprocess
import sys
import unittest


def get_process_out(command, path_to_file, flags):
    args = [command, path_to_file]
    for key in flags.split(' '):
        args.append(key)

    result = subprocess.run(
        args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return result.returncode, result.stdout, result.stderr


class E2E(unittest.TestCase):
    pass


def generate_test_from_file(path_to_file, coolc, coolc_flags, ref, ref_flags):
    def test(self):
        coolc_ret, coolc_stdout, coolc_stderr = get_process_out(
            coolc, path_to_file, coolc_flags)
        ref_ret, ref_stdout, ref_stderr = get_process_out(
            ref, path_to_file, ref_flags)

        self.assertTrue(coolc_ret != 0 and ref_ret !=
                        0 or coolc_ret == 0 and ref_ret == 0)
        self.assertTrue(coolc_stderr == b'')
        if ref_stderr != b'':
            self.assertTrue(coolc_stdout in ref_stderr)
        else:
            self.assertEqual(coolc_stdout, ref_stdout)
    return test


if __name__ == '__main__':
    if len(sys.argv) < 6:
        raise Exception(
            "Must be 6 arguments: reference launch command, reference flags, coolc launch command, coolc flags, folder with tests, file extension!")

    ref_compiler_launch = sys.argv[1]
    ref_flags = sys.argv[2]
    coolc_compiler_launch = sys.argv[3]
    coolc_flags = sys.argv[4]
    folder = sys.argv[5]
    ext = sys.argv[6]

    for filename in os.listdir(folder):
        if filename.endswith(ext):
            test_name = 'test_' + filename.split(".", 1)[0]
            test = generate_test_from_file(
                folder + filename, coolc_compiler_launch, coolc_flags, ref_compiler_launch, ref_flags)
            setattr(E2E, test_name, test)

    unittest.main(argv=['first-arg-is-ignored'])
