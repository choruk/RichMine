import os

for i in range(79, 102):
    print "Finding CEs for {0}...".format(i)
    graph_size = i
    for j in range(100):
        output_file_name = "ce.{0}-{1}.txt".format(graph_size, j)
        os.system("./ce_deconstructor {0} {1}".format(graph_size, output_file_name))
