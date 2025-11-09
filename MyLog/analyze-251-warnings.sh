#!/bin/bash
LOG="tsan-report.log"

echo "=== 251个警告分类统计 ==="
echo "总警告数: $(grep -c "WARNING: ThreadSanitizer" $LOG)"

echo -e "\n按类型分类:"
grep "WARNING: ThreadSanitizer:" $LOG | sort | uniq -c | sort -rn

echo -e "\n按文件分类:"
grep -oP "in \w+ \K\S+:\d+" $LOG | sort | uniq -c | sort -rn | head -20

echo -e "\n按函数分类:"
grep -oP "in \K\w+" $LOG | sort | uniq -c | sort -rn | head -20
