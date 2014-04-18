import sys

argc = len(sys.argv)

filename = "deconstructor.output"
if argc > 1:
    filename = sys.argv[1]

solution_string = ""
with open(filename, 'r') as graph_file:
    file_content = graph_file.read()
    for char in file_content:
        if char == "1" or char == "0":
            solution_string += char

print solution_string