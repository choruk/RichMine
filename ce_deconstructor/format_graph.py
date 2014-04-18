import sys

argc = len(sys.argv)

graph_file_name = "deconstructor.input"
graph_size = 101
if argc > 2:
    graph_file_name = sys.argv[1]
    graph_size = int(sys.argv[2])

graph_file_content = None
with open(graph_file_name, 'r') as graph_file:
    graph_file_content = graph_file.read()

with open(graph_file_name, 'w') as graph_file:
    row_count = 0
    for char in graph_file_content:
        if row_count == graph_size:
            graph_file.write("\n")
            row_count = 0
        if row_count == graph_size - 1:
            graph_file.write("{0}".format(char))
        else:
            graph_file.write("{0} ".format(char))
        row_count += 1