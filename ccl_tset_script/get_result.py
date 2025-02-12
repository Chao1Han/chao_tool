import argparse
import os

parser = argparse.ArgumentParser(description="information for modifing .py")
parser.add_argument("--file_path", required=True, help="input file path, e.g. /path/test.log")
args = parser.parse_args()
files = os.listdir(args.file_path)
save_path = os.path.join(args.file_path, "summary.log")
print(save_path)

with open(save_path, 'w') as save_file:
    for file in files:
        print(file)
        filename = file.rsplit(".", 1)[0]
        if file.endswith("log"):
            file_path = os.path.join(args.file_path, file)
            with open(file_path, 'r') as read_file:
                lines = read_file.readlines()
            save_file.write(str(filename) + ",")
            for i, line in enumerate(lines):
                if "Ran " in line:
                    print(line)
                    save_file.write(str(line.replace("\n", "")) + " " + str(lines[i+2].replace("\n", "")) + ",")
                    break
            if lines:
                save_file.write(lines[-1])
            read_file.close()
save_file.close()
