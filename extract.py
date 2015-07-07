import csv

with open('log.csv', 'r') as csvfile:
	for row in csv.reader(csvfile):
		if row[0] == 'cl.exe' or row[0] == 'link.exe':
			print row[1]
