import sys, time, json
import requests

argc = len(sys.argv)

for i in range(79, 102):
    graph_size = i
    num_submitted = 0
    for j in range(100):
        output_file_name = "ce-2/ce.{0}-{1}.txt".format(graph_size, j)
        solution_string = ""
        with open(output_file_name, 'r') as graph_file:
            file_content = graph_file.read()
            for char in file_content:
                if char == "1" or char == "0":
                    solution_string += char
        req_json = { 'solution' : solution_string, 'clientTimestamp' : int(time.time() * 1000) }
        vault_url = 'https://richcoin.cs.ucsb.edu:8243/vault/1.0.0'
        headers = {
            'Authorization': 'Bearer quT7R6GIF7f2jp1cLc0NLDRdaVMa',
            'Content-Type': 'application/json'
        }
        if num_submitted >= 20:
            print "Submitted {0} requests already, waiting 60 seconds to avoid rate limits".format(num_submitted)
            time.sleep(60)
            num_submitted = 0
            req_json['clientTimestamp'] = int(time.time() * 1000)
        response = requests.post(vault_url, data=json.dumps(req_json), headers=headers, verify=False)
        num_submitted += 1
        print "Just submitted {0} to vault API and got response {1}".format(output_file_name, response.status_code)
        if response.status_code not in range(200,300):
            print response.text
            print solution_string
            print int(time.time() * 1000)
            exit(-1)