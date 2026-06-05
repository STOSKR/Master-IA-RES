# KwsEvalTool

Compute *(interpolated-)precision and recall curve* (**R-P**), *average precision* (**AP**) and/or *mean average precision* (**mAP**) for evaluating keyword spotting performance.

## Compilation and Usage

```bash
gcc -Wall -O4 -o kws-assessment kws-assessment.c

Usage: kws-assessment [options] <table-file>
```

## Input file format

*table-file* must be a plain ASCII text file with the following
information arranged in 4 columns:

```
# Line-ID<string>  Word<string>  If_Relevant<0|1>  Score<float>
  l23-12           potato        1                 0.76
  l23-12           tomato        0                 0.34
  ...              ...           ...               ...
```

## Examples of use

To compute **AP** and **mAP**:
```bash
kws-assessment -t -s -a -m -w egs/keywords_test.lst -l 16376 egs/data_test.dat
```

To generate the corresponding **R-P** curve:
```bash
kws-assessment -t -s -w egs/keywords_test.lst -l 16376 \
               egs/data_test.dat > egs/r-p_data.dat
```
