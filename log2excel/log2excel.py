import re
import os
import xlwt

f = open('raw_log.txt','r',encoding='utf-8')
row = 1
col = 0
wb = xlwt.Workbook(encoding = 'utf-8')
ws1 = wb.add_sheet('first')
for line in f:
    line = line.strip()
    line=re.split('\s{4}',line)
    for n, words in enumerate(line):
        if words == '':
            continue
        words = words.strip()
        if n == 0:
            ws1.write(row, col, words)
            col += 1
            continue
        word = re.split('(ms|us|s)', words)
        for token in word:
            if token == '':
                continue
            ws1.write(row, col ,token)
            col += 1
    row += 1
    col = 0

wb.save("datatable.xls")
